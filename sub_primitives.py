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
wat = re.compile(r'call \$__prim_continuation_delete').sub('drop', wat) #TODO: uncomment this: .sub('continuation_delete', wat)

# Replace noinline calls
wat = re.compile(r'call \$__noinline_hook_control').sub('call $__hook_control', wat)
wat = re.compile(r'call \$__noinline_hook_restore').sub('call $__hook_restore', wat)
wat = re.compile(r'call \$__noinline_hook_copy').sub('call $__hook_copy', wat)
wat = re.compile(r'call \$__noinline_hook_delete').sub('call $__hook_delete', wat)


# Delete the stub imports for the primitives
def delete_import(wat, imp):
    return re.compile(f'\\(import "env" "{imp}" \\(func \\${imp} \\(type \\d+\\)\\)\\)').sub('', wat)

wat = delete_import(wat, '__prim_control')
wat = delete_import(wat, '__prim_restore')
wat = delete_import(wat, '__prim_continuation_copy')
wat = delete_import(wat, '__prim_continuation_delete')
wat = delete_import(wat, '__prim_inhibit_optimizer')

wat = delete_import(wat, '__prim_hook_control_post')
wat = delete_import(wat, '__prim_hook_restore_pre')
wat = delete_import(wat, '__prim_hook_copy_post')
wat = delete_import(wat, '__prim_hook_delete_post')

wat = delete_import(wat, '__noinline_hook_control')
wat = delete_import(wat, '__noinline_hook_restore')
wat = delete_import(wat, '__noinline_hook_copy')
wat = delete_import(wat, '__noinline_hook_delete')

# Delete calls to $__prim_inhibit_optimizer
wat = re.compile(r'call \$__prim_inhibit_optimizer').sub('i32.const 0', wat)

# Replace wasi_snapshot_preview1 namespace with wasi_unstable namespace. Probably because my wasmtime is very far behind master.
wat = re.compile(r'\(import "wasi_snapshot_preview1"').sub('(import "wasi_unstable"', wat)

# Replace hooks with inline Wasm
wat = re.compile(r'call \$__prim_hook_control_post').sub(read_file('hook_control_post.wat') + '\n', wat)
wat = re.compile(r'call \$__prim_hook_restore_pre').sub(read_file('hook_restore_pre.wat') + '\n', wat)
wat = re.compile(r'call \$__prim_hook_copy_post').sub(read_file('hook_copy_post.wat') + '\n', wat)
wat = re.compile(r'call \$__prim_hook_delete_post').sub(read_file('hook_delete_post.wat') + '\n', wat)

# Make sure there are no calls to $__prim_control left
if '$__prim_control' in wat:
    print("Error: indirect control detected. This is not yet supported.", file=sys.stderr)
    exit(1)

with open(sys.argv[1], 'w') as wat_output_f:
    wat_output_f.write(wat)