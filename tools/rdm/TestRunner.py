# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# TestRunner.py
# Copyright (C) 2011 Simon Newton

import datetime
import inspect
import logging
import time

from ola.OlaClient import OlaClient, RDMNack
from ola.RDMAPI import RDMAPI
from ola.testing.rdm import ResponderTest
from ola.testing.rdm.TestState import TestState
from ola.testing.rdm.TimingStats import TimingStats

from ola import PidStore

__author__ = 'nomis52@gmail.com (Simon Newton)'


class Error(Exception):
  """The base error class."""


class DuplicatePropertyException(Error):
  """Raised if a property is declared in more than one test."""


class MissingPropertyException(Error):
  """Raised if a property was listed in a REQUIRES list but it didn't appear in
    any PROVIDES list.
  """


class CircularDependencyException(Error):
  """Raised if there is a circular dependency created by PROVIDES &
     REQUIRES statements.
  """


class DeviceProperties(object):
  """Encapsulates the properties of a device."""
  def __init__(self, property_names):
    object.__setattr__(self, '_property_names', property_names)
    object.__setattr__(self, '_properties', {})

  def __str__(self):
    return str(self._properties)

  def __repr__(self):
    return self._properties

  def __getattr__(self, property):
    if property not in self._properties:
      raise AttributeError(property)
    return self._properties[property]

  def __setattr__(self, property, value):
    if property in self._properties:
      logging.warning('Multiple sets of property %s' % property)
    self._properties[property] = value

  def AsDict(self):
    return dict(self._properties)


class QueuedMessageFetcher(object):
  """This class sends Get QUEUED_MESSAGE until all Ack Timers have expired and
     we get an empty status message or a NACK NR_UNKNOWN_PID.

     QUEUED_MESSAGEs can be triggered a number of ways:
      i) A inline proxy, which responds with ACK_TIMERs to satisfy timing
        requirements.
      ii) A change of state on the responder, i.e. changing the DMX address on
        the front panel.
      iii) A delayed response to a SET command. This can be triggered by delays
        writing to persistent storage.

    It's actually reasonably hard to tell these apart because you can't tell if
    ACK_TIMERS are generated by the responder or intermeditate proxies. In a
    perfect world, devices themselves wouldn't respond with ACK_TIMER to a Get
    QUEUED_MESSAGE and we could use that to 'discover' proxies.

    There is the Proxied Device Flag in the Control field of the discovery
    messages but many implementations don't expose these to the application.
  """
  def __init__(self, universe, uid, rdm_api, wrapper, limit=25):
    self._universe = universe
    self._uid = uid
    self._api = rdm_api
    self._wrapper = wrapper
    # implement some basic endless loop checking
    self._limit = limit
    self._counter = 0
    self._outstanding_ack_timers = 0

    store = PidStore.GetStore()
    self._queued_message_pid = store.GetName('QUEUED_MESSAGE')
    self._status_messages_pid = store.GetName('STATUS_MESSAGES')

  def FetchAllMessages(self):
    self._counter = 0
    if self._FetchQueuedMessage():
      self._wrapper.Run()

  def _FetchQueuedMessage(self):
    if self._counter == self._limit:
      logging.error('Queued message hit loop limit of %d' % self._counter)
      self._wrapper.Stop()
      return

    self._counter += 1
    return self._api.Get(self._universe,
                         self._uid,
                         PidStore.ROOT_DEVICE,  # always sent to the ROOT_DEVICE
                         self._queued_message_pid,
                         self._HandleResponse,
                         ['advisory'])

  def _AckTimerExpired(self):
    self._outstanding_ack_timers -= 1
    self._FetchQueuedMessage()

  def _HandleResponse(self, response, unpacked_data, unpack_exception):
    if not response.status.Succeeded():
      # this indicates a transport error
      logging.error('Error: %s' % response.status.message)
      if (self._outstanding_ack_timers == 0):
        self._wrapper.Stop()
      return

    if response.response_code != OlaClient.RDM_COMPLETED_OK:
      logging.error('Error: %s' % response.ResponseCodeAsString())
      if (self._outstanding_ack_timers == 0):
        self._wrapper.Stop()
      return

    if response.response_type == OlaClient.RDM_ACK_TIMER:
      logging.debug('Got ACK TIMER set to %d ms' % response.ack_timer)
      self._wrapper.AddEvent(response.ack_timer, self._AckTimerExpired)
      self._outstanding_ack_timers += 1
      self._wrapper.Reset()
      return

    # This is now either an ACK or NACK
    # Stop if we get a NR_UNKNOWN_PID to GET QUEUED_MESSAGE
    if (response.response_type == OlaClient.RDM_NACK_REASON and
        response.nack_reason == RDMNack.NR_UNKNOWN_PID and
        response.command_class == OlaClient.RDM_GET_RESPONSE and
        response.pid == self._queued_message_pid.value):
      if (self._outstanding_ack_timers == 0):
        self._wrapper.Stop()
      return

    # Stop if we get a message with no status messages in it.
    if (response.response_type == OlaClient.RDM_ACK and
        response.command_class == OlaClient.RDM_GET_RESPONSE and
        response.pid == self._status_messages_pid.value and
        unpacked_data is not None and
        unpacked_data.get('messages', []) == []):
      if (self._outstanding_ack_timers == 0):
        self._wrapper.Stop()
      if response.queued_messages:
        logging.error(
           'Got a empty status message but the queued message count is %d' %
           response.queued_messages)
      return

    # more remain, keep fetching them
    self._FetchQueuedMessage()


def GetTestClasses(module):
  """Return a list of test classes from a module.

  Args:
    module: The module to search for test classes.

  Returns:
    A list of test classes.
  """
  classes = []
  for symbol in dir(module):
    cls = getattr(module, symbol)
    if not inspect.isclass(cls):
      continue
    base_classes = [
        ResponderTest.OptionalParameterTestFixture,
        ResponderTest.ParamDescriptionTestFixture,
        ResponderTest.ResponderTestFixture,
        ResponderTest.TestFixture
    ]

    if cls in base_classes:
      continue
    if issubclass(cls, ResponderTest.TestFixture):
      classes.append(cls)
  return classes


class TestRunner(object):
  """The Test Runner executes the tests."""
  def __init__(self, universe, uid, broadcast_write_delay, inter_test_delay,
               pid_store, wrapper, timestamp=False):
    """Create a new TestRunner.

    Args:
      universe: The universe number to use
      uid: The UID object to test
      broadcast_write_delay: the delay to use after sending broadcast sets
      inter_test_delay: the delay to use between tests
      pid_store: A PidStore object
      wrapper: A ClientWrapper object
      timestamp: true to print timestamps with each test
    """
    self._universe = universe
    self._uid = uid
    self._broadcast_write_delay = broadcast_write_delay
    self._inter_test_delay = inter_test_delay
    self._timestamp = timestamp
    self._pid_store = pid_store
    self._api = RDMAPI(wrapper.Client(), pid_store, strict_checks=False)
    self._wrapper = wrapper
    self._timing_stats = TimingStats()

    # maps device properties to the tests that provide them
    self._property_map = {}
    self._all_tests = set()  # set of all test classes

    # Used to flush the queued message queue
    self._message_fetcher = QueuedMessageFetcher(universe,
                                                 uid,
                                                 self._api,
                                                 wrapper)

  def TimingStats(self):
    return self._timing_stats

  def RegisterTest(self, test_class):
    """Register a test.

    This doesn't necessarily mean a test will be run as we may restrict which
    tests are executed.

    Args:
      test: A child class of ResponderTest.
    """
    for property in test_class.PROVIDES:
      if property in self._property_map:
        raise DuplicatePropertyException(
            '%s is declared in more than one test' % property)
      self._property_map[property] = test_class
    self._all_tests.add(test_class)

  def RunTests(self, whitelist=None, no_factory_defaults=False, update_cb=None):
    """Run all the tests.

    Args:
      whitelist: If not None, limit the tests to those in the list and their
        dependencies.
      no_factory_defaults: Avoid running the SET factory defaults test.
      update_cb: This is called between each test to update the progress. It
        takes two args, one is the number of test complete, the other is the
        total number of tests.

    Returns:
      A tuple in the form (tests, device), where tests is a list of tests that
      executed, and device is an instance of DeviceProperties.
    """
    device = DeviceProperties(self._property_map.keys())
    if whitelist is None:
      tests_to_run = self._all_tests
    else:
      tests_to_run = []
      matched_tests = []
      for t in self._all_tests:
        if t.__name__ in whitelist:
          tests_to_run.append(t)
          matched_tests.append(t.__name__)
      invalid_tests = whitelist.difference(matched_tests)
      for t in invalid_tests:
        logging.error("Test %s doesn't exist, skipping" % t)

    if no_factory_defaults:
      factory_default_tests = set(['ResetFactoryDefaults',
                                   'ResetFactoryDefaultsWithData'])
      tests_to_run = [test for test in tests_to_run
                      if test.__name__ not in factory_default_tests]

    deps_map = self._InstantiateTests(device, tests_to_run)
    tests = self._TopologicalSort(deps_map)

    logging.debug('Test order is %s' % tests)
    is_debug = logging.getLogger('').isEnabledFor(logging.DEBUG)

    tests_completed = 0
    for test in tests:
      # make sure the queue is flushed before starting any tests
      if update_cb is not None:
        update_cb(tests_completed, len(tests))
      self._message_fetcher.FetchAllMessages()

      # capture the start time
      start = datetime.datetime.now()
      start_time_as_string = '%s ' % start.strftime('%d-%m-%Y %H:%M:%S.%f')
      start_header = ''
      end_header = ''
      if self._timestamp:
        if is_debug:
          start_header = start_time_as_string
        else:
          end_header = start_time_as_string

      logging.debug('%s%s: %s' % (start_header, test, test.__doc__))

      if test.state is TestState.BROKEN:
        test.LogDebug(' Test broken after init, skipping test.')
        continue

      try:
        for property in test.Requires():
          getattr(device, property)
      except AttributeError:
        test.LogDebug(' Property: %s not found, skipping test.' % property)
        tests_completed += 1
        continue

      test.Run()

      # Use inter_test_delay on all but the last test
      if test != tests[-1]:
        time.sleep(self._inter_test_delay / 1000.0)

      logging.info('%s%s: %s' % (end_header, test, test.state.ColorString()))
      tests_completed += 1
    return tests, device

  def _InstantiateTests(self, device, tests_to_run):
    """Instantiate the required tests and calculate the dependencies.

    Args:
      device: A DeviceProperties object
      tests_to_run: The list of test class names to run

    Returns:
      A dict mapping each test object to the set of test objects it depends on.
    """
    class_name_to_object = {}
    deps_map = {}
    for test_class in tests_to_run:
      self._AddTest(device, class_name_to_object, deps_map, test_class)
    return deps_map

  def _AddTest(self, device, class_name_to_object, deps_map, test_class,
               parents=[]):
    """Add a test class, recursively adding all REQUIRES.
       This also checks for circular dependencies.

    Args:
      device: A DeviceProperties object which is passed to each test.
      class_name_to_object: A dict of class names to objects.
      deps_map: A dict mapping each test object to the set of test objects it
        depends on.
      test_class: A class which sub classes ResponderTest.
      parents: The parents for the current class.

    Returns:
      An instance of the test class.
    """
    if test_class in class_name_to_object:
      return class_name_to_object[test_class]

    class_name_to_object[test_class] = None
    test_obj = test_class(device,
                          self._universe,
                          self._uid,
                          self._pid_store,
                          self._api,
                          self._wrapper,
                          self._broadcast_write_delay,
                          self._timing_stats)

    new_parents = parents + [test_class]
    dep_classes = []
    for property in test_obj.Requires():
      if property not in self._property_map:
        raise MissingPropertyException(
            '%s not listed in any PROVIDES list.' % property)
      dep_classes.append(self._property_map[property])
    dep_classes.extend(test_class.DEPS)

    dep_objects = []
    for dep_class in dep_classes:
      if dep_class in new_parents:
        raise CircularDependencyException(
            'Circular dependency found %s in %s' % (dep_class, new_parents))
      obj = self._AddTest(device,
                          class_name_to_object,
                          deps_map,
                          dep_class,
                          new_parents)
      dep_objects.append(obj)

    class_name_to_object[test_class] = test_obj
    deps_map[test_obj] = set(dep_objects)
    return test_obj

  def _TopologicalSort(self, deps_dict):
    """Sort the tests according to the dep ordering.

    Args:
      A dict in the form test_name: [deps].
    """
    # The final order to run tests in
    tests = []

    remaining_tests = [
        test for test, deps in deps_dict.items() if len(deps)]
    no_deps = set(
        test for test, deps in deps_dict.items() if len(deps) == 0)

    while len(no_deps) > 0:
      current_test = no_deps.pop()
      tests.append(current_test)

      remove_list = []
      for test in remaining_tests:
        deps_dict[test].discard(current_test)
        if len(deps_dict[test]) == 0:
          no_deps.add(test)
          remove_list.append(test)

      for test in remove_list:
        remaining_tests.remove(test)
    return tests
