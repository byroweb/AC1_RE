#!/usr/bin/env python3
"""permuter_main.py - run decomp-permuter with the compile step in-process.

    <permuter venv python> tools/loop/permuter_main.py <decomp-permuter dir> <permuter args...>

decomp-permuter compiles each candidate with `bash compile.sh in.c -o out.o`.  On this
Windows PC that costs a bash start-up (~0.28 s), a python start-up and maspsx's own
python start-up (~0.25 s) on top of cc1 (~0.19 s) and as (~0.06 s).  This launcher
replaces src.compiler.Compiler.compile with permute.compile_candidate(), which does the
same thing as compile.sh (`permute.py cc`) inside the permuter process: cc1 and as
are the only subprocesses left.  Everything else is the stock permuter (src.main.main).
"""
import os
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))


def main():
    pd = sys.argv.pop(1)
    sys.path.insert(0, pd)
    sys.path.insert(0, HERE)
    import permute
    from src import compiler, main as pmain
    from src.helpers import try_remove

    metas = {}

    def compile_inproc(self, source, *, show_errors=False):
        d = os.path.dirname(os.path.abspath(self.compile_cmd))
        if d not in metas:
            meta = permute.load_meta(d)
            metas[d] = (meta, permute.symtab(meta))
        meta, syms = metas[d]
        with tempfile.NamedTemporaryFile(prefix="permuter", suffix=".o", delete=False) as f:
            o_name = f.name
        if permute.compile_candidate(meta, syms, source, o_name, show_errors=show_errors or self.show_errors):
            return o_name
        try_remove(o_name)
        return None

    compiler.Compiler.compile = compile_inproc
    pmain.main()


if __name__ == "__main__":
    main()
