﻿import mi

a = mi.Application()
s = mi.Session(a)
q = mi.Query(s, u"root\\cimv2", u"select * from win32_process")
i = q.get_next_instance()

while i is not None:
    #print(dir(i))
    print(i.get_class_name())
    print(i.name)
    print(len(i))
    print(i[u'name'])
    print(i['name'])
    for j in xrange(0, len(i)):
        print(i[j])

    i = q.get_next_instance()

