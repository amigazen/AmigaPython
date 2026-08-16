# Thin re-exports of curated Intuition helpers from builtin amiga.
# For raw LVO access see intuitionlib.py (secondary amigalibs FFI).

from amiga import EasyRequest, CurrentTime, DisplayBeep, MessageBox

__all__ = ["EasyRequest", "CurrentTime", "DisplayBeep", "MessageBox"]
