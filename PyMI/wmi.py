﻿# Copyright 2015 Cloudbase Solutions Srl
# All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.

import re
import six

import mi


class _Method(object):
    def __init__(self, conn, instance, method_name):
        self._conn = conn
        self._instance = instance
        self._method_name = method_name

    def __call__(self, *args, **kwargs):
        return self._conn.invoke_method(
            self._instance, self._method_name, *args, **kwargs)


class _Path(object):
    def __init__(self):
        # TODO: implement
        self.Class = "Msvm_ConcreteJob"


class _Instance(object):
    def __init__(self, conn, instance):
        object.__setattr__(self, "_conn", conn)
        object.__setattr__(self, "_instance", instance)

    def __getattr__(self, name):
        try:
            return _wrap_element(self._conn, *self._instance.get_element(name))
        except mi.error:
            return _Method(self._conn, self, name)

    def __setattr__(self, name, value):
        _, el_type, _ = self._instance.get_element(name)
        self._instance[six.text_type(name)] = _unwrap_element(
            self._instance, el_type, value)

    def associators(self, wmi_association_class=None, wmi_result_class=None):
        return self._conn.get_associators(
            self, wmi_association_class, wmi_result_class)

    def path(self):
        return _Path()

    def path_(self):
        return self._instance.get_path()

    def GetText_(self, text_format):
        return self._conn.serialize_instance(self)


class _Class(object):
    def __init__(self, conn, className, cls):
        self._conn = conn
        self.class_name = six.text_type(className)
        self._cls = cls

    def __call__(self, *argc, **argv):
        fields = ""
        for i, v in enumerate(argc):
            if i > 0:
                raise ValueError('Invalid argument')
            if not isinstance(v, list):
                raise ValueError('Invalid argument')
            for f in v:
                if fields:
                    fields += ", "
                # TODO: sanitize input
                fields += f
        if not fields:
            fields = "*"

        where = ""
        for k, v in argv.items():
            if not where:
                where = " where "
            else:
                where += ", "
            where += k + " = '%s'" % v

        wql = (u"select %(fields)s from %(className)s%(where)s" %
               {"fields": fields,
                "className": self.class_name,
                "where": where})

        return self._conn.query(wql)

    def new(self):
        return self._conn.new_instance_from_class(self)

    def watch_for(self, notification_type="operation",
                  delay_secs=1, fields=[], **where_clause):
        # TODO: implement
        pass


class _Connection(object):
    def __init__(self, computer_name=".", ns="root/cimv2",
                 protocol=mi.PROTOCOL_WMIDCOM):
        self._ns = six.text_type(ns)
        self._app = mi.Application()
        self._session = self._app.create_session(
            computer_name=six.text_type(computer_name),
            protocol=six.text_type(protocol))

    def __del__(self):
        self._session = None
        self._app = None

    def __getattr__(self, name):
        with self._session.get_class(
                ns=self._ns, className=six.text_type(name)) as q:
            cls = q.get_next_class().clone()
            return _Class(self, name, cls)

    def _get_instances(self, op):
        l = []
        i = op.get_next_instance()
        while i is not None:
            l.append(_Instance(self, i.clone()))
            i = op.get_next_instance()
        return l

    def query(self, wql):
        wql = wql.replace("\\", "\\\\")
        with self._session.exec_query(
                ns=self._ns, query=six.text_type(wql)) as q:
            return self._get_instances(q)

    def get_associators(self, instance, wmi_association_class=None,
                        wmi_result_class=None):
        if wmi_association_class is None:
            wmi_association_class = u""
        if wmi_result_class is None:
            wmi_result_class = u""

        with self._session.get_associators(
                ns=self._ns, instance=instance._instance,
                assoc_class=six.text_type(wmi_association_class),
                result_class=six.text_type(wmi_result_class)) as q:
            return self._get_instances(q)

    def invoke_method(self, instance, method_name, *args, **kwargs):
        cls = instance._instance.get_class()
        params = self._app.create_method_params(
            cls, six.text_type(method_name))

        for i, v in enumerate(args):
            _, el_type, _ = params.get_element(i)
            params[i] = _unwrap_element(params, el_type, v)
        for k, v in kwargs.items():
            _, el_type, _ = params.get_element(k)
            params[k] = _unwrap_element(params, el_type, v)

        with self._session.invoke_method(
                instance._instance, six.text_type(method_name), params) as op:
            l = []
            r = op.get_next_instance()
            for i in six.moves.range(0, len(r)):
                l.append(_wrap_element(self, *r.get_element(i)))
            return tuple(l)

    def new_instance_from_class(self, cls):
        return _Instance(
            self, self._app.create_instance_from_class(
                cls.class_name, cls._cls))

    def serialize_instance(self, instance):
        with self._app.create_serializer() as s:
            return s.serialize_instance(instance._instance)

    def get_class(self, class_name):
        with self._session.get_class(ns=self._ns, className=class_name) as op:
            cls = op.get_next_class()
            if cls:
                return _Class(self, class_name, cls.clone())

    def get_instance(self, class_name, key):
        c = self.get_class(class_name)
        key_instance = self.new_instance_from_class(c)
        for k, v in key.items():
            key_instance._instance[six.text_type(k)] = v
        try:
            with self._session.get_instance(
                    self._ns, key_instance._instance) as op:
                instance = op.get_next_instance()
                if instance:
                    return _Instance(self, instance.clone())
        except mi.error:
            return None


def _wrap_element(conn, name, el_type, value):
    if isinstance(value, mi.Instance):
        if el_type == mi.MI_INSTANCE:
            return _Instance(conn, value)
        elif el_type == mi.MI_REFERENCE:
            return value.get_path()
        else:
            raise Exception(
                "Unsupported instance element type: %s" % el_type)
    if isinstance(value, (tuple, list)):
        if el_type == mi.MI_REFERENCEA:
            return tuple([i.get_path() for i in value])
        elif el_type == mi.MI_INSTANCEA:
            return tuple([_Instance(conn, i) for i in value])
        else:
            return tuple(value)
    else:
        return value


def _unwrap_element(instance, el_type, value):
    if value is not None:
        if el_type == mi.MI_REFERENCE:
            return WMI(value)._instance
        elif el_type == mi.MI_INSTANCE:
            return value._instance
        if el_type == mi.MI_REFERENCEA:
            l = []
            for item in value:
                l.append(_unwrap_element(value, mi.MI_REFERENCE, item))
            return tuple(l)
        elif el_type == mi.MI_INSTANCEA:
            l = []
            for item in value:
                l.append(_unwrap_element(value, mi.MI_INSTANCE, item))
            return tuple(l)
        elif el_type == mi.MI_STRING:
            return six.text_type(value)
        else:
            return value


def _parse_moniker(moniker):
    PROTOCOL = "winmgmts:"
    computer_name = '.'
    namespace = None
    path = None
    class_name = None
    key = None
    m = re.match("(?:" + PROTOCOL + r")?//([^/]+)/([^:]*)(?::(.*))?", moniker)
    if m:
        computer_name, namespace, path = m.groups()
        if path:
            m = re.match("([^.]+).(.*)", path)
            if m:
                key = {}
                class_name, kvs = m.groups()
                for kv in kvs.split(","):
                    m = re.match("([^=]+)=\"(.*)\"", kv)
                    name, value = m.groups()
                    key[name] = value
            else:
                class_name = path
    else:
        namespace = moniker
    return (computer_name, namespace, class_name, key)


def WMI(moniker="root/cimv2", privileges=None):
    computer_name, ns, class_name, key = _parse_moniker(
        moniker.replace("\\", "/"))
    conn = _Connection(computer_name=computer_name, ns=ns)
    if not class_name:
        return conn
    else:
        return conn.get_instance(class_name, key)
