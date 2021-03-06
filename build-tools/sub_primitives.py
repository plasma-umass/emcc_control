#!/usr/bin/env python3
import sys
import re

def read_file(path):
    with open(path, 'r') as f:
        return f.read()

wat = read_file(sys.argv[1])

# Fix up the syntax of the function table
wat = re.compile(r'\(elem \(;\d+;\) \(i32\.const 1\) func ').sub(r'(elem (i32.const 1) ', wat)

# Extract the function table
func_table_search = re.compile(r'\(elem \(;0;\) \(i32\.const 1\) ([^\)]*)\)').search(wat)
if func_table_search is not None:
    func_table_str = re.compile(r'\(elem \(;0;\) \(i32\.const 1\) ([^\)]*)\)').search(wat).group(1)
    func_table = func_table_str.split()
else:
    func_table = []

# print(func_table)

# Replace $__prim_control calls
def replace_control(match):
    func_name = func_table[int(match.group(1)) - 1]
    return f"control {func_name}"
wat = re.compile(r'i32\.const (\d+)\s+call \$__prim_control').sub(replace_control, wat)

# Replace $__prim_restore calls
wat = re.compile(r'call \$__prim_restore').sub('restore', wat)
# Replace $__prim_continuation_copy calls
wat = re.compile(r'call \$__prim_continuation_copy').sub('continuation_copy', wat)
# Replace $__prim_continuation_delete calls
wat = re.compile(r'call \$__prim_continuation_delete').sub('continuation_delete', wat)
# Replace $__prim_prompt_begin calls
wat = re.compile(r'call \$__prim_prompt_begin').sub('(prompt', wat)
# Replace $__prim_prompt_end calls
wat = re.compile(r'call \$__prim_prompt_end').sub(')', wat)

# Delete the stub imports for the primitives
def delete_import(wat, imp):
    return re.compile(f'\\(import "env" "{imp}" \\(func \\${imp} \\(type \\d+\\)\\)\\)').sub('', wat)

wat = delete_import(wat, '__prim_control')
wat = delete_import(wat, '__prim_restore')
wat = delete_import(wat, '__prim_continuation_copy')
wat = delete_import(wat, '__prim_continuation_delete')
wat = delete_import(wat, '__prim_prompt_begin')
wat = delete_import(wat, '__prim_prompt_end')
wat = delete_import(wat, '__prim_inhibit_optimizer')

wat = delete_import(wat, '__prim_get_shadow_stack_ptr')
wat = delete_import(wat, '__prim_set_shadow_stack_ptr')

wat = re.compile(r'\(import "env" "asyncify_start_unwind"').sub('(import "asyncify" "start_unwind"', wat)
wat = re.compile(r'\(import "env" "asyncify_stop_unwind"').sub('(import "asyncify" "stop_unwind"', wat)
wat = re.compile(r'\(import "env" "asyncify_start_rewind"').sub('(import "asyncify" "start_rewind"', wat)
wat = re.compile(r'\(import "env" "asyncify_stop_rewind"').sub('(import "asyncify" "stop_rewind"', wat)


# wat = delete_import(wat, 'asyncify_start_unwind')
# wat = delete_import(wat, 'asyncify_stop_unwind')
# wat = delete_import(wat, 'asyncify_start_rewind')
# wat = delete_import(wat, 'asyncify_stop_rewind')


# Delete calls to $__prim_inhibit_optimizer
wat = re.compile(r'call \$__prim_inhibit_optimizer').sub('i32.const 0', wat)

# Replace wasi_snapshot_preview1 namespace with wasi_unstable namespace. Probably because my wasmtime is very far behind master.
wat = re.compile(r'\(import "wasi_snapshot_preview1"').sub('(import "wasi_unstable"', wat)

# Replace hooks with inline Wasm
wat = re.compile(r'call \$__prim_get_shadow_stack_ptr').sub('global.get 0', wat)
wat = re.compile(r'call \$__prim_set_shadow_stack_ptr').sub('global.set 0', wat)

# Make sure there are no calls to $__prim_control left
if '$__prim_control' in wat:
    print("Error: indirect control detected. This is not yet supported.", file=sys.stderr)
    exit(1)

with open(sys.argv[1], 'w') as wat_output_f:
    wat_output_f.write(wat)