import os, math

s = os.popen( "find / -printf '%f'", "r" ).read()
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
<<<<<<< HEAD
s = 0
for c in chl[ 1:64 ]:
    s += c.freq
=======
res = ""
for c in chl[ 0:64 ]:
>>>>>>> b1b8d534af43c71a8c3a7a1eb014c3263c0034cc
    if c.name >= 32 and c.name < 128:
        print c.freq, chr( c.name )
        res += chr( c.name )
    else:
        print c.freq, c.name

print res
print len( res )
for sep in [ 16, 32, 64, 128, 256 ]:
    si = sum( c.freq for c in chl[0:sep] )
    ss = sum( c.freq for c in chl[sep:]  )
    sb = math.log( sep ) / math.log( 2 )
    print "fixed size", sep, si, ss, si * sb + ss * ( sb * ( sep < 256 ) + 8 )

def cs( d, sep ):
    bs = 1 + math.log( sep ) / math.log( 2 )
    res = bs
    while d > sep:
        res += bs
        d /= sep
    return res
    
for sep in [ 8, 16, 32, 64, 128, 256 ]:
    si = 0
    for i in range( 256 ):
        si += chl[ i ].freq * cs( i, sep )
    print "compressed", sep, si

#429093 e
#315169 a
#308639 .
#276552 t
#268511 c
#251651 o
#250763 s
#250505 i
#231507 p
#221345 n
#215832 d
#209018 r
#184984 l
#172835 m
#155518 f
#150129 b
#149461 g
#149003 0
#148488 _
#125802 h
#119858 1
#112440 3
#110762 2
#110470 6
#108788 u
#106438 4
#100369 8
#97800 -
#97011 5
#88302 7
#87674 9
#71650 k
#57346 x
#48830 y
#46435 v
#29359 w
#20276 z
#19797 j
#17573 M
#15275 S
#15056 C
#13535 A
#13464  
#12301 L
#12008 D
#11686 I
#10422 :
#10291 E
#10203 q
#10077 P
#10005 T
#8538 O
#6412 B
#5904 R
#5101 K
#4656 N
#4261 F
#4066 G
#3495 H
#2665 U
#2637 W
#2466 V
#2123 X
#1941 Q
