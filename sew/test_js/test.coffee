a = new PI8 0
console.log a.size_in_bits
console.log a.val
a.val = 17
console.log a.val

class MyView extends View 
    onchange: ->
        console.log "pouet"
        
v = new MyView a
a.val = 10
