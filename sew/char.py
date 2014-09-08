import os
s = os.popen( "find /", "r" ).read()
t = [ 0 for i in range( 256 ) ]
for c in s:
    t[ ord( c ) ] += 1
print len( s )

class Ch:
    def __init__( self, freq, name ):
        self.freq = freq
        self.name = name
    def ___str___( self ):
        return str( self.freq ) + " " + str( self.name )
    
    
chl = [ Ch( t[ i ], i ) for i in range( 256 ) ]
chl = sorted( chl, key = lambda x: - x.freq  )
for c in chl[ 1:64 ]:
    if c.name >= 32 and c.name < 128:
        print c.freq, chr( c.name )
    else:
        print c.freq, c.name
    
#4259403 e
#3000689 s
#2979835 r
#2630552 a
#2426584 i
#2267751 t
#2225290 o
#2145789 c
#2110400 l
#1820934 n
#1636404 u
#1632342 d
#1407394 -
#1325629 .
#1268955 h
#1250062 m
#1148081 p
#1070914 0
#937947 3
#890866 10
#877578 f
#851064 g
#850201 1
#826798 x
#762659 2
#660515 b
#637182 _
#513651 v
#448346 k
#418316 4
#363740 6
#346924  
#322737 8
#308169 7
#307157 5
#293465 S
#284303 y
#276345 9
#225539 P
#189304 C
#176644 M
#170692 w
#167911 E
#150568 D
#138009 T
#126236 A
#117469 N
#109864 R
#94277 L
#84507 j
#79012 z
#70149 I
#68672 G
#62303 F
#61903 O
#60549 :
#52093 q
#47526 U
#46239 V
#40388 B
#24766 H
#24349 J
#22343 Y
