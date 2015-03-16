# dep Toto.js

t = new Toto
console.log t


# ptrs = for i in [ 0 .. 10 ]
#     asm_mod.allocate( 16 ).ptr
# console.log asm_mod.get_occupation()
# for ptr in ptrs
#     asm_mod.free ptr, 16
# console.log asm_mod.get_occupation()

# a = new PI8 asm_mod.allocate 1
# # console.log a.size_in_bits
# # console.log a.val
# # a.val = 17
# # console.log a.val
# 
# class MyView extends View 
#     onchange: ->
#         console.log a.val
#         
# v = new MyView a
# a.val = 10
# 
# Model.__timeout = setTimeout ( -> a.val = 20 ), 2000
