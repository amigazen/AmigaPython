# Core interpreter / new-style class / slot wiring tests.

from __future__ import print_function

import sys
import traceback
from AmigaTests.support import check, expect_raises, skip


def test_platform():
    check("sys.platform is amiga", sys.platform == "amiga", repr(sys.platform))
    check("version 2.7", sys.version_info[:2] == (2, 7))
    check("builtin amiga", "amiga" in sys.builtin_module_names)
    check("sys.path non-empty", len(sys.path) > 0)


def test_site_quitters():
    import __builtin__
    check("quit present", hasattr(__builtin__, "quit"))
    check("exit present", hasattr(__builtin__, "exit"))
    expect_raises("quit() SystemExit", SystemExit, quit)
    expect_raises("exit() SystemExit", SystemExit, exit)


def test_newstyle_init():
    class Q(object):
        def __init__(self, name):
            self.name = name

    check("Quitter-style __init__", Q("ok").name == "ok")

    class Base(object):
        def __init__(self, n):
            self.n = n

    class Child(Base):
        def __init__(self, n, m):
            Base.__init__(self, n)
            self.m = m

    c = Child(1, 2)
    check("inherited __init__", c.n == 1 and c.m == 2)

    class D(object):
        def __new__(cls, x):
            obj = object.__new__(cls)
            obj.x = x
            return obj

        def __init__(self, x):
            self.inited = True

    d = D(7)
    check("__new__+__init__", d.x == 7 and d.inited is True)


def test_slots_ops():
    class V(object):
        def __init__(self, n):
            self.n = n

        def __add__(self, o):
            return V(self.n + o.n)

        def __eq__(self, o):
            return isinstance(o, V) and self.n == o.n

        def __lt__(self, o):
            return self.n < o.n

        def __repr__(self):
            return "V(%r)" % (self.n,)

        def __hash__(self):
            return hash(self.n)

        def __nonzero__(self):
            return self.n != 0

        def __call__(self, x):
            return self.n + x

    check("__add__", (V(1) + V(2)).n == 3)
    check("__eq__", V(3) == V(3))
    check("__lt__", V(1) < V(2))
    check("__nonzero__", (not V(0)) and bool(V(1)))
    check("__repr__", repr(V(9)) == "V(9)")
    check("__hash__", hash(V(4)) == hash(4))
    check("__call__", V(10)(5) == 15)

    class Seq(object):
        def __init__(self, data):
            self.data = list(data)

        def __len__(self):
            return len(self.data)

        def __getitem__(self, i):
            return self.data[i]

        def __contains__(self, v):
            return v in self.data

    s = Seq([10, 20, 30])
    check("seq len/item/contains", len(s) == 3 and s[1] == 20 and 20 in s)

    class Map(object):
        def __init__(self):
            self.d = {"a": 1}

        def __len__(self):
            return len(self.d)

        def __getitem__(self, k):
            return self.d[k]

        def __setitem__(self, k, v):
            self.d[k] = v

    m = Map()
    m["b"] = 2
    check("map get/set", m["a"] == 1 and m["b"] == 2 and len(m) == 2)


def test_property():
    class T(object):
        def __init__(self):
            self._v = 0

        def get_v(self):
            return self._v

        def set_v(self, v):
            self._v = v

        v = property(get_v, set_v)

    t = T()
    t.v = 5
    check("property", t.v == 5)


def test_core_types():
    check("list/dict/tuple/set",
          [1, 2][1] == 2 and {"k": 1}["k"] == 1 and (1, 2)[0] == 1 and
          set([1, 1, 2]) == set([1, 2]))
    check("int/long/float/str",
          1 + 2 == 3 and (1L << 40) > 0 and abs(0.5 * 2 - 1) < 1e-9 and
          "ab" + "c" == "abc")
    check("listcomp/genexp",
          [x * x for x in range(4)] == [0, 1, 4, 9] and
          list(x for x in range(3)) == [0, 1, 2])


def test_exceptions_traceback():
    try:
        raise ValueError("boom")
    except ValueError, e:
        check("raise/except", str(e) == "boom")
    else:
        check("raise/except", False)

    def boom():
        raise RuntimeError("x")

    try:
        boom()
    except RuntimeError:
        lines = traceback.format_exc()
        check("traceback.format_exc",
              "RuntimeError" in lines and "boom" in lines)


def test_gc_weakref():
    import gc
    try:
        import weakref
    except ImportError:
        skip("weakref", "not available")
        return

    class Node(object):
        pass

    a = Node()
    b = Node()
    a.other = b
    b.other = a
    del a, b
    n = gc.collect()
    check("gc.collect", isinstance(n, (int, long)))

    o = Node()
    r = weakref.ref(o)
    check("weakref alive", r() is o)
    del o
    gc.collect()
    check("weakref cleared", r() is None)
