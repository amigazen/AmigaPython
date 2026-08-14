# Tier A C accelerators: _collections, itertools, _functools, _random,
# _bisect, _heapq, _csv, _symtable.

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


def test_bisect():
    b = require_import("_bisect")
    if not b:
        return
    data = [1, 3, 3, 5, 7]
    check("_bisect.bisect_left", b.bisect_left(data, 3) == 1)
    check("_bisect.bisect_right", b.bisect_right(data, 3) == 3)
    check("_bisect.bisect", b.bisect(data, 4) == 3)
    copy = list(data)
    b.insort_left(copy, 3)
    check("_bisect.insort_left", copy == [1, 3, 3, 3, 5, 7])
    copy2 = list(data)
    b.insort_right(copy2, 3)
    check("_bisect.insort_right", copy2 == [1, 3, 3, 3, 5, 7])

    try:
        import bisect
    except ImportError, e:
        skip("bisect", str(e))
        return
    check("bisect.bisect", bisect.bisect([10, 20, 30], 25) == 2)
    seq = [10, 20, 30]
    bisect.insort(seq, 25)
    check("bisect.insort", seq == [10, 20, 25, 30])


def test_heapq():
    h = require_import("_heapq")
    if not h:
        return
    heap = [5, 1, 3, 8, 2]
    h.heapify(heap)
    check("_heapq.heapify", heap[0] == 1)
    check("_heapq.heappop", h.heappop(heap) == 1 and heap[0] == 2)
    h.heappush(heap, 0)
    check("_heapq.heappush", heap[0] == 0)
    check("_heapq.heappushpop", h.heappushpop(heap, -1) == -1)
    check("_heapq.heapreplace", h.heapreplace(heap, 9) == 0)
    check("_heapq.nlargest", h.nlargest(3, [5, 1, 9, 3, 7]) == [9, 7, 5])
    check("_heapq.nsmallest", h.nsmallest(2, [5, 1, 9, 3]) == [1, 3])

    try:
        import heapq
    except ImportError, e:
        skip("heapq", str(e))
        return
    heap2 = [9, 2, 7]
    heapq.heapify(heap2)
    check("heapq.heappushpop", heapq.heappushpop(heap2, 1) == 1)
    check("heapq.merge",
          list(heapq.merge([1, 4], [2, 3])) == [1, 2, 3, 4])


def test_csv():
    c = require_import("_csv")
    if not c:
        return
    check("_csv reader/writer",
          hasattr(c, "reader") and hasattr(c, "writer"))
    check("_csv Error", hasattr(c, "Error"))
    check("_csv dialects", hasattr(c, "list_dialects"))

    try:
        import csv
        from cStringIO import StringIO
    except ImportError, e:
        skip("csv", str(e))
        return

    buf = StringIO()
    w = csv.writer(buf)
    w.writerow(["a", "b,c", "d"])
    w.writerow([1, 2, 3])
    text = buf.getvalue()
    check("csv writer quotes", '"b,c"' in text or "b,c" in text)

    buf.seek(0)
    rows = list(csv.reader(buf))
    check("csv reader", rows[0] == ["a", "b,c", "d"])
    check("csv reader ints as str", rows[1] == ["1", "2", "3"])

    buf2 = StringIO("name,age\nAda,36\n")
    dicts = list(csv.DictReader(buf2))
    check("csv DictReader",
          len(dicts) == 1 and dicts[0]["name"] == "Ada" and
          dicts[0]["age"] == "36")

    out = StringIO()
    dw = csv.DictWriter(out, fieldnames=["x", "y"])
    dw.writeheader()
    dw.writerow({"x": "p", "y": "q"})
    check("csv DictWriter", out.getvalue().startswith("x,y"))


def test_symtable():
    st = require_import("_symtable")
    if not st:
        return
    check("_symtable.symtable", hasattr(st, "symtable"))
    check("_symtable constants",
          hasattr(st, "USE") and hasattr(st, "DEF_LOCAL"))

    try:
        import symtable
    except ImportError, e:
        skip("symtable", str(e))
        return

    src = ("def outer():\n"
           "    x = 1\n"
           "    def f(a):\n"
           "        return x + a\n"
           "    return f\n")
    table = symtable.symtable(src, "<test>", "exec")
    check("symtable top", table is not None)
    ids = table.get_identifiers()
    check("symtable identifiers", "outer" in ids)
    check("symtable has_children", table.has_children())

    outers = [c for c in table.get_children() if c.get_name() == "outer"]
    check("symtable outer child", len(outers) == 1)
    if not outers:
        return
    outer = outers[0]
    check("symtable outer locals", "x" in outer.get_locals())

    funcs = [c for c in outer.get_children() if c.get_name() == "f"]
    check("symtable function child", len(funcs) == 1)
    if funcs:
        fn = funcs[0]
        check("symtable is_nested/function",
              fn.get_type() == "function")
        check("symtable params", "a" in fn.get_parameters())
        check("symtable locals", "a" in fn.get_locals())
        # Nested f closes over outer's x -> free/cell.
        frees = fn.get_frees()
        check("symtable frees", "x" in frees)


def test_builtin_names_present():
    import sys
    names = sys.builtin_module_names
    for name in ("_collections", "itertools", "_functools", "_random",
                 "datetime", "zipimport",
                 "_symtable", "_bisect", "_heapq", "_csv"):
        check("builtin " + name, name in names, repr(names[:12]))
