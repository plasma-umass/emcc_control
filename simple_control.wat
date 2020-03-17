(module
  (type (;0;) (func (param i64 i64)))
  (type (;1;) (func))
  (type (;2;) (func (result i32)))
  (type (;3;) (func (param i32) (result i32)))
  (type (;4;) (func (param i32)))
  (type (;5;) (func (param i32 i32) (result i32)))
  (type (;6;) (func (param i64 i32) (result i64)))
  
  
  
  (func $__wasm_call_ctors (type 1)
    nop)
  (func $handler (type 0) (param i64 i64)
    local.get 0
    i64.const 42
    restore)
  (func $handler_two (type 0) (param i64 i64)
    local.get 0
    i64.const 987
    restore)
  (func $__garbage_please_delete_me_handler (type 1)
    i64.const 0
    i64.const 1
    i32.const 1
    i32.const 0
    i32.const 0
    select
    call_indirect (type 0))
  (func $__garbage_please_delete_me_handler_two (type 1)
    i64.const 0
    i64.const 1
    i32.const 2
    i32.const 0
    i32.const 0
    select
    call_indirect (type 0))
  (func $the_main (type 2) (result i32)
    i64.const 1234
    control $handler
    i64.const 5678
    control $handler_two
    i64.add
    i32.wrap_i64)
  (func $main (type 5) (param i32 i32) (result i32)
    i32.const 0)
  (func $stackSave (type 2) (result i32)
    global.get 0)
  (func $stackAlloc (type 3) (param i32) (result i32)
    global.get 0
    local.get 0
    i32.sub
    i32.const -16
    i32.and
    local.tee 0
    global.set 0
    local.get 0)
  (func $stackRestore (type 4) (param i32)
    local.get 0
    global.set 0)
  (func $__growWasmMemory (type 3) (param i32) (result i32)
    local.get 0
    memory.grow)
  (table (;0;) 5 5 funcref)
  (memory (;0;) 256 256)
  (global (;0;) (mut i32) (i32.const 5243904))
  (global (;1;) i32 (i32.const 1024))
  (export "memory" (memory 0))
  (export "__garbage_please_delete_me_handler" (func $__garbage_please_delete_me_handler))
  (export "__garbage_please_delete_me_handler_two" (func $__garbage_please_delete_me_handler_two))
  (export "the_main" (func $the_main))
  (export "main" (func $main))
  (export "_start" (func $__wasm_call_ctors))
  (export "__data_end" (global 1))
  (export "stackSave" (func $stackSave))
  (export "stackAlloc" (func $stackAlloc))
  (export "stackRestore" (func $stackRestore))
  (export "__growWasmMemory" (func $__growWasmMemory))
  (elem (i32.const 1) $handler $handler_two $__wasm_call_ctors $main)
  (data (;0;) (i32.const 1024) "\a0\04P"))
