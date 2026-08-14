# Tier A C accelerators: _collections, itertools, _functools, _random.

from __future__ import print_function

from AmigaTests.support import check, skip, require_import


def test_collections_deque_defaultdict():
    # Pure-Python collections needs _collections for deque/defaultdict.
    try:
        from collections import deque, defaultdict, Counter, namedtuple
    except ImportError, e:
        check("import collections", False, str(e))
        return
    check("import collections", True)

    d = deque([1, 2, 3])
    d.append(4)
    d.appendleft(0)
    check("deque ops", list(d) == [0, 1, 2, 3, 4] and d.pop() == 4)

    dd = defaultdict(int)
    dd["a"] += 1
    dd["a"] += 1
    check("defaultdict", dd["a"] == 2 and dd["b"] == 0)

    c = Counter("aab")
    check("Counter", c["a"] == 2 and c["b"] == 1)

    Point = namedtuple("Point", "x y")
    p = Point(1, 2)
    check("namedtuple", p.x == 1 and p.y == 2 and p == (1, 2))


def test_itertools_basics():
    it = require_import("itertools")
    if not it:
        return
    check("count", list(it.islice(it.count(10), 3)) == [10, 11, 12])
    check("cycle", list(it.islice(it.cycle([1, 2]), 5)) == [1, 2, 1, 2, 1])
    check("chain", list(it.chain([1], [2, 3])) == [1, 2, 3])
    check("izip", list(it.izip([1, 2], "ab")) == [(1, "a"), (2, "b")])
    check("repeat", list(it.repeat(7, 3)) == [7, 7, 7])
    check("ifilter", list(it.ifilter(lambda x: x % 2, [1, 2, 3, 4])) == [1, 3])
    check("imap", list(it.imap(lambda x: x * x, [1, 2, 3])) == [1, 4, 9])
    check("product", list(it.product([0, 1], repeat=2)) ==
          [(0, 0), (0, 1), (1, 0), (1, 1)])
    check("combinations", list(it.combinations("ABC", 2)) ==
          [("A", "B"), ("A", "C"), ("B", "C")])
    check("groupby",
          [(k, list(g)) for k, g in it.groupby("AAABB")] ==
          [("A", ["A", "A", "A"]), ("B", ["B", "B"])])


def test_functools_basics():
    ft = require_import("_functools")
    if not ft:
        return
    check("_functools has partial", hasattr(ft, "partial"))
    add = ft.partial(lambda a, b: a + b, 10)
    check("partial", add(5) == 15)

    # High-level functools module (wraps _functools).
    try:
        import functools
    except ImportError, e:
        skip("import functools", str(e))
        return
    check("import functools", True)
    check("functools.partial", functools.partial(pow, 2)(8) == 256)
    if hasattr(functools, "reduce"):
        check("functools.reduce", functools.reduce(lambda a, b: a + b, [1, 2, 3]) == 6)


def test_random_module():
    # _random is the C accelerator; random.py is the public API.
    rmod = require_import("_random")
    if not rmod:
        return
    check("_random.Random", hasattr(rmod, "Random"))
    rnd = rmod.Random()
    rnd.seed(12345)
    a = rnd.random()
    rnd.seed(12345)
    b = rnd.random()
    check("_random deterministic", a == b and 0.0 <= a < 1.0)

    try:
        import random
    except ImportError, e:
        skip("import random", str(e))
        return
    check("import random", True)
    random.seed(99)
    x = random.random()
    random.seed(99)
    y = random.random()
    check("random.seed", x == y)
    check("random.randint", 1 <= random.randint(1, 6) <= 6)
    check("random.choice", random.choice([10, 20, 30]) in (10, 20, 30))
    seq = [1, 2, 3, 4]
    random.shuffle(seq)
    check("random.shuffle", sorted(seq) == [1, 2, 3, 4])


def test_builtin_names_present():
    import sys
    names = sys.builtin_module_names
    for name in ("_collections", "itertools", "_functools", "_random"):
        check("builtin " + name, name in names, repr(names[:12]))
