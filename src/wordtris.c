/*  This file is a part of gtkboard, a board games system.
    Copyright (C) 2003, Arvind Narayanan <arvindn@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

*/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <gdk/gdkkeysyms.h>


#include "game.h"
#include "../pixmaps/alpha.xpm"
#include "flwords.h"


#define WORDTRIS_CELL_SIZE 36

#define WORDTRIS_NUM_PIECES 27

#define WORDTRIS_BOARD_WID 4
#define WORDTRIS_BOARD_HEIT 14

#define WORDTRIS_LEN 4

#define WORDTRIS_EMPTY 0
#define WORDTRIS_WILDCARD 27

char wordtris_colors[9] = {0xd7, 0xd7, 0xd7, 0xd7, 0xd7, 0xd7, 0, 0, 0xff};
char wordtris_highlight_colors[9] = {0x0, 0x0, 0xff, 0, 0, 0, 0, 0, 0};

static char *smiley_xpm[] = 
{
	"36 36 2 1",
	"  c none",
	". c #ffff77",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"         ...              ...       ",
	"        .....            .....      ",
	"         ...              ...       ",
	"                                    ",
	"                                    ",
	"                  ..                ",
	"                  ..                ",
	"                  ..                ",
	"                  ..                ",
	"                  ..                ",
	"                  ..                ",
	"                  ..                ",
	"                                    ",
	"                                    ",
	"         ..                ..       ",
	"          ..              ..        ",
	"           ...          ...         ",
	"            ....      ....          ",
	"              ..........            ",
	"                ......              ",
	"                  ..                ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
};

static char **wordtris_pixmaps [] = 
{
	char_A_grey_36_36_xpm,
	char_B_grey_36_36_xpm,
	char_C_grey_36_36_xpm,
	char_D_grey_36_36_xpm,
	char_E_grey_36_36_xpm,
	char_F_grey_36_36_xpm,
	char_G_grey_36_36_xpm,
	char_H_grey_36_36_xpm,
	char_I_grey_36_36_xpm,
	char_J_grey_36_36_xpm,
	char_K_grey_36_36_xpm,
	char_L_grey_36_36_xpm,
	char_M_grey_36_36_xpm,
	char_N_grey_36_36_xpm,
	char_O_grey_36_36_xpm,
	char_P_grey_36_36_xpm,
	char_Q_grey_36_36_xpm,
	char_R_grey_36_36_xpm,
	char_S_grey_36_36_xpm,
	char_T_grey_36_36_xpm,
	char_U_grey_36_36_xpm,
	char_V_grey_36_36_xpm,
	char_W_grey_36_36_xpm,
	char_X_grey_36_36_xpm,
	char_Y_grey_36_36_xpm,
	char_Z_grey_36_36_xpm,
	smiley_xpm,
};



// TODO: change this so that we only need to specify pixmaps for individual squares

static const int lava_xpm_header_size = 253+1;
static char * lava_xpm_header[] = 
{
"144 504 253 2",
"  	c #d7d7d7",
". 	c #FFD800",
"+ 	c #FFAC00",
"@ 	c #FFA800",
"# 	c #FF8000",
"$ 	c #FF0600",
"% 	c #830000",
"& 	c #310000",
"* 	c #150000",
"= 	c #1F0000",
"- 	c #2D0000",
"; 	c #270000",
"> 	c #210000",
", 	c #1B0000",
"' 	c #230000",
") 	c #1D0000",
"! 	c #0B0000",
"~ 	c #010000",
"{ 	c #090000",
"] 	c #2F0000",
"^ 	c #3B0000",
"/ 	c #630000",
"( 	c #8B0000",
"_ 	c #930000",
": 	c #8D0000",
"< 	c #CF0000",
"[ 	c #FF6600",
"} 	c #FF9E00",
"| 	c #FF4C00",
"1 	c #FF7A00",
"2 	c #FF8E00",
"3 	c #FF7600",
"4 	c #FF5800",
"5 	c #FF6200",
"6 	c #FFB600",
"7 	c #FFDA00",
"8 	c #FF8C00",
"9 	c #FF9400",
"0 	c #FFCE00",
"a 	c #FFD200",
"b 	c #FF9A00",
"c 	c #FF7800",
"d 	c #FFF000",
"e 	c #FFDE00",
"f 	c #FFAE00",
"g 	c #FF8400",
"h 	c #FF7000",
"i 	c #FF8200",
"j 	c #FFA400",
"k 	c #FFB000",
"l 	c #FFA000",
"m 	c #FF9800",
"n 	c #FFAA00",
"o 	c #FFEA00",
"p 	c #FFD400",
"q 	c #FF7400",
"r 	c #FF6E00",
"s 	c #FFCC00",
"t 	c #FFBC00",
"u 	c #FF7200",
"v 	c #FFC400",
"w 	c #FFEE00",
"x 	c #FFA200",
"y 	c #FF9200",
"z 	c #FFB400",
"A 	c #FFA600",
"B 	c #FF9000",
"C 	c #FF8800",
"D 	c #FFD000",
"E 	c #FF9C00",
"F 	c #FF1000",
"G 	c #D90000",
"H 	c #FF0E00",
"I 	c #FF5000",
"J 	c #FF8600",
"K 	c #FF9600",
"L 	c #FF7E00",
"M 	c #FF6C00",
"N 	c #FF6000",
"O 	c #FF4200",
"P 	c #FF2600",
"Q 	c #FF4400",
"R 	c #FFE600",
"S 	c #FF5C00",
"T 	c #E50000",
"U 	c #B30000",
"V 	c #A90000",
"W 	c #7F0000",
"X 	c #550000",
"Y 	c #5F0000",
"Z 	c #890000",
"` 	c #BF0000",
" .	c #F30000",
"..	c #ED0000",
"+.	c #AD0000",
"@.	c #7D0000",
"#.	c #DD0000",
"$.	c #FF2E00",
"%.	c #FF2C00",
"&.	c #FF1200",
"*.	c #370000",
"=.	c #110000",
"-.	c #070000",
";.	c #0F0000",
">.	c #130000",
",.	c #190000",
"'.	c #170000",
").	c #450000",
"!.	c #4D0000",
"~.	c #510000",
"{.	c #570000",
"].	c #9F0000",
"^.	c #FF3200",
"/.	c #FF4800",
"(.	c #D70000",
"_.	c #AF0000",
":.	c #E90000",
"<.	c #BD0000",
"[.	c #A30000",
"}.	c #FF8A00",
"|.	c #FF5200",
"1.	c #E30000",
"2.	c #9D0000",
"3.	c #FF3000",
"4.	c #FF0200",
"5.	c #FFD600",
"6.	c #910000",
"7.	c #C10000",
"8.	c #FF1E00",
"9.	c #FF3C00",
"0.	c #FF1800",
"a.	c #FF0800",
"b.	c #F10000",
"c.	c #E70000",
"d.	c #FF1400",
"e.	c #F50000",
"f.	c #970000",
"g.	c #FF4E00",
"h.	c #FF2A00",
"i.	c #FF1A00",
"j.	c #FF0400",
"k.	c #F70000",
"l.	c #F90000",
"m.	c #EF0000",
"n.	c #FF4000",
"o.	c #FF2800",
"p.	c #FF2200",
"q.	c #DB0000",
"r.	c #A50000",
"s.	c #D10000",
"t.	c #C50000",
"u.	c #670000",
"v.	c #6F0000",
"w.	c #9B0000",
"x.	c #AB0000",
"y.	c #DF0000",
"z.	c #8F0000",
"A.	c #A70000",
"B.	c #C90000",
"C.	c #FFB800",
"D.	c #FFE000",
"E.	c #FF0C00",
"F.	c #6D0000",
"G.	c #290000",
"H.	c #410000",
"I.	c #750000",
"J.	c #EB0000",
"K.	c #D50000",
"L.	c #5B0000",
"M.	c #000000",
"N.	c #050000",
"O.	c #390000",
"P.	c #770000",
"Q.	c #C30000",
"R.	c #B70000",
"S.	c #B50000",
"T.	c #FFB200",
"U.	c #A10000",
"V.	c #FFBA00",
"W.	c #FF5400",
"X.	c #FF0000",
"Y.	c #CB0000",
"Z.	c #870000",
"`.	c #D30000",
" +	c #4B0000",
".+	c #250000",
"++	c #FF6A00",
"@+	c #FFC000",
"#+	c #CD0000",
"$+	c #FF3600",
"%+	c #FF3400",
"&+	c #2B0000",
"*+	c #3F0000",
"=+	c #730000",
"-+	c #590000",
";+	c #850000",
">+	c #790000",
",+	c #3D0000",
"'+	c #B90000",
")+	c #330000",
"!+	c #030000",
"~+	c #430000",
"{+	c #690000",
"]+	c #FFBE00",
"^+	c #FFCA00",
"/+	c #FD0000",
"(+	c #FF3A00",
"_+	c #BB0000",
":+	c #490000",
"<+	c #7B0000",
"[+	c #810000",
"}+	c #6B0000",
"|+	c #650000",
"1+	c #E10000",
"2+	c #FF5E00",
"3+	c #B10000",
"4+	c #0D0000",
"5+	c #FFC600",
"6+	c #710000",
"7+	c #FFC800",
"8+	c #C70000",
"9+	c #470000",
"0+	c #FFE400",
"a+	c #FF0A00",
"b+	c #FF5A00",
"c+	c #FFC200",
"d+	c #FF5600",
"e+	c #FF1600",
"f+	c #990000",
"g+	c #FF7C00",
"h+	c #5D0000",
"i+	c #530000",
"j+	c #FF2400",
"k+	c #FFE200",
"l+	c #FF6400",
"m+	c #610000",
"n+	c #4F0000",
"o+	c #950000",
"p+	c #FF3E00",
"q+	c #FF2000",
"r+	c #350000",
"s+	c #FB0000",
"t+	c #FF6800",
"u+	c #FF1C00",
"v+	c #FF4A00",
"w+	c #FFEC00",
"x+	c #FF4600",
"y+	c #FF3800",
"z+	c #FFDC00",
"A+	c #FFE800",
"B+	c #FFF600",
"C+	c #FFF200",
"D+	c #FFF400",
};

static char *lava_xpm_data[] =
{
". + @ # $ % & * = - - ; > , = ' ) ! ~ { = ] ^ / ( _ : ( < [ } [ | 1 2 3 4 5 6 7 6 8 3 3 9 0 a b c 9 a d e f g h h i j k j l m 8 9 n a o d p 9 q r q 9 s t r u v w o p 6 x x x x } y 9 f z A A z n B # 3 c C n D E F G H I h 1 J y E K L h r M N O P Q f R a @ m S T U V W X Y Z `  ...+.@.W #.8 ",
"} $.%.&._ *.=.-.{ ;.>.,.) ,., > , ! ~ -.'.) ) ] ).!.~.{.].^./.(._.:. .<.[.:.}.z |.1.[.2. .B @ 3.< 4.m 5.} %.` _ 6.7.8.9.0.a.b.G c.d.5 A D K e.2.: f...g C d.$.n . 6 C g.h.i.j.k.l.m.k.n.I o.3./.p.q.U ].r.s.%.} 1 t.u.v.Z w.x.7.G ..y.+.z._ w.A.B.4.|.C.D.k 5 n.E.V W Z F.*.G.H.I.B.J.V {.).+.1 ",
"i m.J.K.L.=.~ M.M.~ N.>.= = ) = '.-.~ N.=.>.! ;.= - *.O.P.E.3.Q.Z R.< S.+.l.B T.n.x.H.] U.r V.S :.$ 9 C.W.< Y ] - u.(.X.Y.r.6.% Z.w.B.%.l J `. += .+( I g 9.++@+@ |.$.d.4.q.: F.( V #+$.I $.$+%+y.W ).G.& F.T i M V *.; &+& *+{.u.=+/ *.] -+Z +.T 9.# k a s l [ X.].I.;+>+,+'.) H.6.#.'+{.)+w.q ",
"C e.c.(./ * !+M.M.M.{ , G.; .+; , { ~ N.=.=.-.{ ) O.~+,+{+m.O d.Y.q.%.5 h J ]+^+C /+v.O.Z.(+6 t y 9 @+x o._+=+!.:+<+< :.S.[+}+{+}+|+P...2 f O _+|+*.>+3.x f v 6 | 1+(.1. .(.>+~+P.q.P g + l A }.o.S.-+' ' L.q.# M V *..+- *+X {+P.>+/ ).F.y.9.5 3 J h 2+9 p 7 2 j.3+_ U.A.P.,+.+&+/ ` t.F.~+r.c ",
"b $.8.E.U. +; >.N.N.>.; - ' > G.' 4+!+! >.=.-.{ *.Z.+.V R.X.n.3.1+J.h 5+6 i h C x ++y.6+{+t.Q m 7+7+b 4 4.8+].<+>+r.B.Y._.P. +9+~.!.F.k.C C.A h 8.Q.B.O t 0+v I R.W _+J.H a+'+% < b+@ D s + c+^+j I K.W }+: l.8 r x.O.- !.W r.U _+_+2._ ..N # ++d+%+a.e+[ v D.8  .+.f+]._+R.;+!.&  +2.7.[+~._.g+",
"@+B L 4 e.].F.~+= '..+& ; * * .+G.'.;.* '.;.N.>.=+i.M h M q 1 Q 8+Y.q 6 W.#+w.s.I 9 n.+.L.h+2.i.} 9 F <.V [.6.W f.8+Q.x.r.Z.i+9+~.X Z i.J 2 l ]+f g+q j . 0+B <  +u.k.5 C i Q j+M c+k+k+x 9.W.9 v c+# (+8.p.4 + q +.^ ,+>+_+Y.<._+S.].t.Q u &.+.Z.<+_.9.l D p g m.2.% P.( +.x.W  +).% U Z.X A.++",
"z 8 l+3.1+U.;+v.!.O.*.] , { ! ' & ; ) ,.>.4+>.*+B.# V.j T.7+c+5 x.V l+L #+*+) !.1.L r `.Y O.X (.i M V ). +/ m+h+6.1+K.f.2.2.F.-+-+L.6.i.g i 1 j l 3 3 } 6 c+u _.:+w.| } b f 6 6 s 7+6 5+i 1+S.b.N V.p 7+t C.v p 3 A.^ n+6.+.% L.i+i+{.A.(+9.w.- >.' o+l+V.K M 3.#+z.I.|+h+=+w.U.6+ +<+U ].>+V %.",
"n.X.S.Z u.~.X }+6+{.*.) ! !+{ = & ] = ;.4+; }+8+W.V.b Q N n s h x.x.5 N 6.* ~ , Z p+L q+_+f++.&.9 l+z.> &+:+:+H.>+1+:.A.U B.6.h+9+).v.s.(+M 1 ++i.` 8+i.| u r X.` E.c S 0.d+z e v l+$+8 h R.X |+Q.|.V.e k+D.0+7 r 2.,+m+o+% H.'.=.>..+% 0.H F.=.!+) ].M E j+U ( [+@.{+h+{.L.@._.f+h+}++.< Y.B.y.",
"_.;+X O.- G.]  +P.F.r+;.!+M.N.* .+; * -.=.}+F g+V.@+[ s+$ S T.}.s+s+i 5 ( =.M.-.9+G N # M [ q E @+5 Z = G.~+ +9+=+K.s+1+4.p.:.Z.9+O.9+u.].e.%+H w.H.i+R./+q+g.|./.l+N ..w.b.1 7+8 1+B.[ N f+.+) ~.#+Q u 3 c g 8 (+( ).6+6./ .+{ -.{ ,.6+j.a+P.> * ] r.t+A o.I.& !.}+|+X i+Y @._+B.% {.I.x.t.Q.` ",
"F./ -+~.9+O.- O.u.<+ +'.-.-.! >.> &+' , *+8+L V.m t+u+1+..3.2 k }.B @+M 6.* M.~ , }+J.| M C ]+a f (+>+, ' ^ ).9+u.S...$ %.d+Q Y.-+*.*.r+!.2.1+t.|+> G.P.Q.1+e.j+p+O u+V ;+1.2+6 h _.V 2+S ( >.N..+=++._.U.U.t.s+q.{+H.F.% X ) N.N.-.* h+c.0._.{.H.:+_ $+A 4 ( &+^ L.u.{.:+-+[+B. .S.-+9+L.}+W [.",
"W v.F.P.v.!.r+& !.=+Y - ,.'.,.) - X W _ #+W.V.K j+ .y.#+s.m.o.2+# f ^+L S.G.N.-.>.*.P.+.` X.8 f d+:.|+> ' & )+& 9+;+R.G  .%.4 l.6+H.*+*+n+: `.K.( ).- ).[+_._+J.a.d.H G `.a.v+@ J y.t.l+2+( >.!+= X {+9+&+G.-+3+` @./ @.% X = N.~ !+>.n+8+p.k.A.[+}+=+1.i [ U.*.^ {.}+|+!. +{+U a.T =+*+)+] n+o+",
"A.Z.W ( [+-+*.&+*./ =+!.- ) ,.= n+Q.(+t+g C.@+t+e.(.t.V V x.+.<.:.p+}.2 j.i+=.,.&+,+~.-+{.V ++B 8.< ( L. +~+*..+- Y z.x.8+&.d+&.w.=+>+>+=+Z._+T K.].v.-+=+w.'+T J.T $ 0.0.4.d.2 z /.a.3 ++[.; >.- Y |+*.,.> -+Q./+c.`.` : i+> -.~ 4+)+P.s.0.F 1+8+U.<+7.r M 3+^ )+~.v.=+L.).).Z H F 6.).G., ~+].",
"_+( ;+6.% |+9+r+O.Y Z W X - > ~+U W.C.6 8 g u %. .Y.2.Z f+2._ _ 3+ .I m Q @.= ; *.,+*+O.r+o+l+B %.&.H m.Y._.: i+- H.v.6.` u+d+e+Y.Q.(.K.3+( 6.Q.:.T #+3+V U `./+K.2.+.K.s.U.+.n.z C 9.8 B X.w.% 6.[.% ,+.+:+( 1.^.4 I H _.u.)+>.* *+( s.l.F s+q.s.t.2.B.r 3 Q.~+&+:+6+>+Y ~+r+I.F $.x.H.,.;.*+V ",
"S.6+I.6.[+/  +^ ^ L.o+'+r.W Z < |.V.c+M a+/+&.X.K.].;+o++._.r.3+s.F c t q R.~+& ] > ,.>.> o+M k i 2 l B 3 t+n.G {+^ :+|+f.l.| P q.8+`.s.` +.x.Q.1+:.k.a.X.m.4.E.7.{+-+u.Y 9+Y c.B @ 3 + V.q O $+9.o.(.}+i+<+f+x.(.d.h.a.G _.P.i+u.+...l.../+$ :.(.G ` G q K a+Z.~.{.I.[+{+~+&+h+e.n.#+).>.>.:+U ",
"+.-+L.( @.n+O.] &+*+v.+.`. .O J t c+c j.t.s.1.#+[.( w.S.'+r.o++.G 8.B p f q+[+,+G.=.N.{ ^ #+2 7 p 7 p V.T.+ m 5 J.W X {.=+R.u+%+c.2.<+6+@.f+` 1.b.T y./+e+d.a+:.o+~.,+:+~.h+;+e.K D 5+7+} 4 p+Q b+u v+q.].f.>+!.:+|+<+o+S.B.8+8+:.H X.B.r.S.< #+< .. .e+K 7+i H A.v.m+}+|+*+) O.7.Q a+P.~+i+Z q.",
"f.).H.W @.).' * ;.* - u.#+Q f v b N a+t.S._+S.U._ r.Q.t.A.@.v.f.<.T d+c+^+d+A.:+- >.{ - r.I 5+o w+o s j l y y n g+0.s._.6.;+7.F  .w.X O.^ X ;+<.c.m.8+_.` s.Q.2.}+ + +u.Z R.s+W.k R o 6 p+K.<.< $ [ 8 ^.'+I.~+,.{ 4+,.)+-+Z t.e.a.c.3+[+{+/ v.% U H I g @+p k [ 4.Z H.).L.).,.) _ x+/.Q.2.Y.k.e+",
"=+*+*+<+% :+, { ~ ~ { O.Q.q s E 8.`._.w.f+w.w.f.[._+8+x.@.L.|+f+3+'+d.} T.^.w.L.9+] G.P.3.6 a c+c+5.D @+v ]+t 5.p k }.d+J.W P.t.c.R.<+X ).:+Y <+r.q.G U 2.o+W {+Y m+6+f.K.o.L f v 0+o m m.[+6+f+m.q 2 a+=+] * N.M.M.-.'.G.!.;+r.o+F.-+!.O..+' H.r.O l j i [ I v+y+<.~+& i+ +) * Z.W.t+G S.1.e...",
"v.L.Y Z f+u.&+! M.M.~ ' _.1 @+5 `.6.( : _ f+o+o+].A.f+@./ m+W x._+_+X.B x %.< _+S.r.U...C 5+B d+W.r L 9 @+z+0+o 0+R A+^+M T U.r.#+8+3+U.6.Z Z.>+>+2.7.8+_+].I.|+6+( x.1+9.K a k+e w+o b  .[+u.:  .J q R.- -.~ M.M.~ ! ) - ,+:+~+- ' ] ] ,.-.N..+r.[ j n.Q.z.6.(.^.X.6+r+~.n+.+> o+N [ R.@.A._.w.",
"% I.<+2._+[.{.'.~ M.M.* _ t+b 0.V : o+A.+.].z.z.: <+F.{+{+F.[+[.Q.q.q+E @+2 r ++++[ ++2 v j %+e.1+t.'+1.Q E 5.0+. A+B+C+0 2 O /+k.4.j.b.` [.o+Z.% <+P.( x.+.;+v.@.x...n.y 5+R w+0+k+e z x+7.I.Z X.2 ++w.= !+M.M.~ 4+&+~.{+u.9+= { ;.; ; ;.!+-.&+_.r c Q.^ * > v.X.3.'+~.i+|+Y W :.# N f+~.@.U.V ",
"[.F.u.: t.(._ )+-.M.M.=.( 5 }.a.3+V <.B.U : [+;+I.m+{+P.F.Y v.U.y.p.S y 9 u 5 5 t+3 b 5+D L s+7.2.m+,+-+S.%+n 5.p e w+D+d z+6 8 # C }.I #+<+m+}+: Z -+ +<+U.f.( w.q.Q K z c+z+0 y u r q t+0.U 3+q+} q U ^ ,.! ! = ).% _+(.8+Z *+, , &+; >.>.& {+1+# 5 6.* M.N.r+` (+k.6+|+A.m.y+1 x Q W & {.f.s.",
"Q.|+^ X w.y.Y./ ) ! { ,.6.t+@ /.e.(.s._+( }+6+6+/ }+% @.m+m+( #+e+W.N (+k.S.f+].S.m.|.l } 9.Y.U [.F.X @.< %.++q h u 9 0 0 9 r N l+g V.y ../ & ~+[+[.6+H.6+8+:.m.a+v+l ^+5+D . }.J.f.6._+8.I 3.%.++z c 7.-+*+)+,+F.r.K.c.m.l.T x.<+m+n+*+O.-+w.1+(+l [ z.* M.M.,.Z %.p.2.[.e+++L q I y.!.>..+v.(.",
":.W H.,+F.B...f+^ , ,.G.f.++7+9 P `.A.Z.u./ v.{+/ P.W Y ~.@.Q.j.p+|.8.s.f.Y r+& |+#.d+M y+:.Q.1+k.1+T 0.O %.c.V z.w.b.}.}. .w.Z o+:.J V.x+_ *.r+P.'+2.m+2.y+i }.B + 0 7 . D.c+%+P.) * *+3+^.2+5 g k 2+[.n+ +L.@.+.t.S.z.W A.:.H H b.t.r.U.B.l.F p+E c U ; !+M.;.P.%.| k.q+q ++F t.f+-+, !+=.Y 1.",
"a+U.{+Y }+_.b._+n+) ,..+@.y+V.l F 2.P.{+/ {+F.u.{+I.|+*+i+x.s+$+4 %+1._.o+=+).] I.E.g.s+A.].Q.k.^.d+5 4 h.G [+,+= - ].[ q _+r+>.) >+9.t 9 j.f.u._ m.X.#+4.8 t 8 ++[ M h y v }.Y.r+N.~ >.h+< 0.n.8 b &.|+' - ~.[+f.% X - = ,+Z q.8.v+W.v+p.k.s._.y.++9 a.-+4+!+, Z /.2 u i r s+W ).& ,.-.M.=.{+/+",
"u+V P.=+>+[...s.L.= '.,  +y.8 l o.s.R.f+I.Y -+Y {+u.~.).% e.v+l+n...S.f+% <+u.X o+u+$.U >+_ r.A.s.&.i.#.f+=+h+^ > - 6.I C s+~.! 4+-+E.n 0 b /.b.a.M y g+}.C.2 X.U.: ( 2.e.i h 3+- ! { ) {+q.j+W.x 3 S.&+-.4+&+X L.,+' ;.N.4+; X r.8.c b I t.{+ +W 0.y S x.*.& F.1+g 7+6 g+4.<+- , ,.=.-.~ =.F.d.",
"o.A.v.P.( 7.F 4.2.u./ |+W y.h + 2 3 l+d.f.X ~./ {+h+L.Z.b.S g+/.:.x.f.I.i+|+( +.m.g.v+T _+S.: X !.u.v.~.r+O.!.:+^ ,+P.d.2 W.].)+^ x.W.v 0+k+^+} l @+C.g t+l+$.].O.,.>.] U.| g+$ Z n+).6+#.v+q K c+[ ( >.M.~ '.,+*+.+'.4+!+M.!+* F.u+8 J d.@.] = ~+` N 2 $.7.t.j+L V.t 1 a+W G.;.4+=.>.4+-.=.6+j+",
"9.J.< q.e.$.t+2+i.s+s+l.l.E.h.I l+i n l+<.u.6+6.A.+.8+&.u 2 I #.f+z.W n+G.~+2.0.3 x # q+q.V 6+,+= '., ) ) &+,+,+r+& :+'+2+C 8._+< /.k t A v e . ]+2 Q q.2.f.f.=+,+'.! .+[+j.M u P < U 1+(+2+/.u 6 [ Z =.M.~ * O.^ = * =.N.M.M.=.W v+y h.].).= * )+: u+g 9 g+C k f g+y+y.% ] { N.! =.>.;.{ =.6+%.",
"u u c L J B C l+%+H 4.l.J.t.f+Z f...i 8 a.R.Q.j.p+2+3 y B | G <+|+=+h+r+' ^ ].O m L y+K.Z.{.*+& > =.4+>.> )+r+.+'.=., m+k.3 B 3 i k n g.E.(+t+r Q J.2.i+.+> O.{.{.^ ' & }+_+q+q [ &.:.j.o.a.1+3.l l+( =.M.~ * *.^ > * =.N.M.~ > A.t+q < X ] ' ' 9+z.1.%.5 C c+t |.B.% u.).,.-.! * ,.* ;.-.;.F.j+",
"A K q l+S x+&.:.7.: I.=+|+:+G.'..+( g.@ i I | h g+i B g+P R.|+,+~+Y / / @.U.J.2+N m.6.i+G.* * ' G.= >.;.) O.O.) { !+N..+Z.0.g+2 3 l+j+V Y v.Z o+: {+9+.+;.-.* *.X -+:+9+{+f.K.%.n.a+m./+l.7.Y.d+C.N Z.=.M.~ =.& ^ .+,.=.N.M.{ n+e.# /.f.^ )+*+i+P.w.+.U <.4.2 } a.m+- *+^ ,.4+* ,.* 4+N.~ ! h+k.",
"[ &.R.z.Z.<+6+P.F.~+&+> '.! N.N.* Y ..2+K f j u $.i.j+/+U.~.- .+& L.2.c.3.S 1 J ^.o+^ , ! N.-.'.G.- .+'.) ,+ +*+- ,.4+, -+Y.e+X.7.o+{+- ;.4+>.) ; - &+> '.>.=.) *.n+n+!.}+f+B.$ p.e+E.4.`.f+s.g+T.%.u.4+M.~ ;.G.*.&+) >.N.!+> f.v+g+e./ - ^ {+f.+.[.% }+m++.M 6 9.>+&+^ ^ ) * '.;.-.~ M.M.! !.Q.",
"G >+&+>.;.=.= 9+L.H.> 4+!+M.~ ! ,.)+/ V i.8 9 p+X.k.1.[.L.] = , .+/ :.S c u 5 $+#+-+G..+' = ,.'.> ] r+] ]  +6+Z.W / i+Y z.`.G : ).> ;.!+M.M.!+{ =.) &+)+*.)+.+, > r+^ ^ L._ #+e.H p.P /+U ( y.i i B.)+!+M.N.>..+)+- = >.{ ,.u./+1 | [.O.; H.>+U.2.@.h+~.n+o+I c+g #+X 9+H.' ,.=.N.M.M.M.!+>.9+2.",
"f+,+{ M.M.M.4+] n+~+, N.M.M.N.=.'.>.>.] f+$+r n.&.T U.Y *.> >.* )+Z p.u $.K.V W n+- ] 9+h+Y  +O.H.L.P.Z.;+Z.o+r.V +.` K.c.c.3+{.= -.M.M.~ -.4+* ,.' *. +X ~.*+&+= ; )+*.9+P.U < :.j+| $.1.8+e+y t+6.* ~ ~ { * > ] &+) ,., ~.G S 5 :.Y &+] ).|+6+h+n+n+-+L.Z.8.k ]+n.U./ n+- ,.4+!+M.M.!+4+) :+o+",
"A.!.>.!+M.M.N.= H.,+'.~ M.!+4+* =.N.M.! -+ .W.y+y.Z ).' , * 4+.+v.Y.3.b+e.=+O..+, = r+h+% Z P.6+o+8+..j.$ T _.@.6+6.` K.#+x.F.] ;.!+!+-.4+* ,., ,.= & 9+i+X n+~+)+] *.,+!.v.w.3+s.H p+9.p.p.4 @ [ Z =.!+{ >.,.= &+G.) G.-+<.| q 4.@.r+; ] O.9+X -+|+v.I.{+@. .2 0 1 B.>+Y r+'.-.~ M.!+4+* ) *+Z ",
"G 6.~.; 4+!+N.) *+^ * !+!+4+* * ! !+M.N.^ 3+E.:.>+- ;.{ =.* ,.n+_.:.a.%.:./ > =.;., r+{.I.P.>+: <.1.J.m.s+b.+.i+&+,+/ }+-+~+- '.! -.4+* ,.,.* ;.! ! =.) ' G.*.i+P.% =+m+u.P.z.U.3+t.s.#. .E.Q l l+( >.N.;.'.* , G.G.G./ K.x+i n.2.*.) '.* ,.)+m+W W v.Y ~.m+Q.5 @+# `.[+u.*+) ! !+!+4+* * =.- }+",
"$ s+K._ n+> >.&+9+O.'.{ ;.,., =.{ -.-.4+] <+Q.U n+>.-.{ * ; !.w.1+K.7.l.J.>+& > > ]  +m+F.}+@.f+U V % P._ 7.x.{..+&+H.*+&+> > = , = ; &+; > ,.* =.=.>.>.* '.- <+k.$+e+y.Y.t.R.r.U.2.: ( 2.3+X.C [ f.' '.' G..+&+*.*.~.8+| 3 | #.Y ' ,.'.* = :+Z f.{+H.,+n+6+Q.S ]+i s.>+u.X *.) ;.;.'.,.=.4+' ~.",
"^.b+W.H U Y & ^  +^ ' , ) = , >.;.>.'.) )+|+A.U F.,+& - *+=+8+u+%+X.1.q+o.`.[.w.2.V _+Q.t.R.S.G ..s.V w.S.m.s+t.2.w.x.V o+: z.z.z.o+w.f.6.: Z Z Z Z Z Z.[+P.Z T 3 @+z K J h p+a.e.m.q.B.B.G j+} }...f.: f.2.].2.: P.U.h.J t+u+G [.: ( Z Z o+7.b.y.: u.( _+1.j+9 D L s.6+|+F.}+{.H.*.)+)+] - O.~.",
"@ C.b (+q.Z.:+H. +9+^ - = ,.=.4+>.= ' ; )+-+U.B.3+o+% I.: J.5 x l i i } y N 5 q c 1 i g g+| 8.4 J i q h 3 B b J 3 3 1 1 u h M r r u q h h r r r r M M ++4 (+9.3 5+w d w+A+z+@+l m K 8 i # J A 5.D 9 u h u 3 u b+i.T /+r C.f 2 1 h M M M M u J B [ a...(+1 y + 5.7 1 q.>+|+@.].x.f+I.m+{+v.6+6+I.",
};


typedef struct
{
	int score;
	int lives;
} Wordtris_state;

static int total_lives = 10;


static int wordtris_wordcmp (const void *p1, const void *p2)
{
	return strcmp ((char *)p1, *(char **)p2);
}

static void wordtris_init ();
int wordtris_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
int wordtris_getmove_kb (Pos *, int, byte **, int **);
void wordtris_free ();
ResultType wordtris_who_won (Pos *, Player , char **);
int wordtris_animate (Pos *pos, byte **movp);
static void *wordtris_newstate (Pos *, byte *);

Game Wordtris = { WORDTRIS_CELL_SIZE, WORDTRIS_BOARD_WID, WORDTRIS_BOARD_HEIT, 
	WORDTRIS_NUM_PIECES, 
	wordtris_colors, NULL, wordtris_pixmaps, 
	"Wordtris", "Word games", 
	wordtris_init};

/* This list was produced using

  $  egrep -x [a-z]{4} /usr/share/dict/words | perl -e 'while (<>) {foreach $i ((0 .. 3)) {$count{substr($_,$i,1)}++}} foreach $c (("a" .. "z")) {print $count{$c}, "\n"}'

  Perl totally rocks
*/

static float charcounts[26] = 
{
	629,
	223,
	205,
	302,
	746,
	152,
	186,
	182,
	395,
	30,
	167,
	460,
	234,
	338,
	503,
	272,
	6,
	395,
	622,
	404,
	276,
	63,
	173,
	24,
	125,
	28,
};

static char ** wordtris_getbgxpm ()
{
	int i;
	static char *xpm[lava_xpm_header_size + WORDTRIS_CELL_SIZE * WORDTRIS_BOARD_HEIT];
	static char emptyline[WORDTRIS_CELL_SIZE * WORDTRIS_BOARD_WID * 2];
	for (i=0; i<WORDTRIS_CELL_SIZE * WORDTRIS_BOARD_WID * 2; i++)
		emptyline[i] = ' ';
	for (i=0; i<lava_xpm_header_size; i++)
		xpm[i] = lava_xpm_header[i];
	for (i=0; i<WORDTRIS_CELL_SIZE*WORDTRIS_BOARD_HEIT; i++)
		xpm[i+lava_xpm_header_size] = emptyline;
	for (i=WORDTRIS_CELL_SIZE; i<2*WORDTRIS_CELL_SIZE; i++)
		xpm[WORDTRIS_CELL_SIZE * (WORDTRIS_BOARD_HEIT-3)+i+lava_xpm_header_size] = lava_xpm_data [i-WORDTRIS_CELL_SIZE];
	return xpm;
}

// returns hamming distance
static int diffcnt (const char *w1, const char *w2, int len)
{
	int cnt = 0, i;
	for (i=0; i<len; i++)
		if (w1[i] != w2[i])
			cnt++;
	return cnt;
}

static void wordtris_set_init_pos (Pos *pos)
{
	int i, j;
	const char *word;
	for (i=0; i<board_wid * board_heit; i++)
		pos->board[i] = 0;
	// make sure we have a neighbor
	while (1)
	{
		word = flwords [random() % num_flwords];
		for (i=0; i<WORDTRIS_LEN; i++)
			pos->board [i] = word[i] - 'a' + 1;
		for (j=0; j<num_flwords; j++)
			if (diffcnt (word, flwords[j], WORDTRIS_LEN) == 1)
				return;
	}
}

static void wordtris_set_init_render (Pos *pos)
{
	pos->render [0] = RENDER_HIGHLIGHT1;
}

static void wordtris_init ()
{
	int i;
	static char emptyline[WORDTRIS_BOARD_WID * WORDTRIS_CELL_SIZE];
	static char dottedline[WORDTRIS_BOARD_WID * WORDTRIS_CELL_SIZE];
	game_single_player = TRUE;
	game_set_init_pos = wordtris_set_init_pos;
	game_free = wordtris_free;
	game_getmove = wordtris_getmove;
	game_getmove_kb = wordtris_getmove_kb;
	game_who_won = wordtris_who_won;
	game_scorecmp = game_scorecmp_def_dscore;
	game_animation_time = 2000;
	game_animate =  wordtris_animate;
	game_stateful = TRUE;
	game_state_size = sizeof (Wordtris_state);
	game_newstate = wordtris_newstate;
	game_bg_pixmap = wordtris_getbgxpm ();
	game_highlight_colors = wordtris_highlight_colors;
	game_set_init_render = wordtris_set_init_render;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Wordtris\n"
		"Single player game\n"
		"Status: Partially implemented (playable)\n"
		"URL: "GAME_DEFAULT_URL("wordtris");
	game_doc_rules = 
		"Press Ctrl+G to start the game.\n\n"
		" Click one of the letters of the word at the bottom and type the letter on one of the falling blocks to change that letter to the new letter. The new word must be legal.\n"
		"The smiley face acts as a wildcard and can be used as an arbitrary letter.\n"
		"\n"
		"You get a point for every new word you make. You have a total of ten lives. You lose a life when a block falls to the bottom row. The game ends when you lose all your lives.";
	game_doc_strategy = 
		"Try to replace rarely occuring letters with more commonly occuring	letters\n";
}

static int wordtris_curx = 0, wordtris_cury = 0;

ResultType wordtris_who_won (Pos *pos, Player to_play, char **commp)
{
	static char comment[32];
	int i;
	*commp = comment;
	for (i=0; i<WORDTRIS_LEN; i++)
		if (pos->board [2 * board_wid + i] != WORDTRIS_EMPTY 
				&& pos->state && ((Wordtris_state *)pos->state)->lives == 0)
		{
			snprintf (comment, 32, "Game over. Score: %d", 
					pos->state ? ((Wordtris_state *)pos->state)->score: 0);
			return RESULT_WON;
		}
	snprintf (comment, 32, "Score: %d;  Lives: %d", 
					pos->state ? ((Wordtris_state *)pos->state)->score: 0,
					pos->state ? ((Wordtris_state *)pos->state)->lives: total_lives);
	return RESULT_NOTYET;
}

void *wordtris_newstate (Pos *pos, byte *move)
{
	static Wordtris_state state;
	int i, score = 0, count = 0, died = 0;
	for (i=0; move[3*i] >= 0; i++)
	{
		if (move[3*i+1] == 0)
			score = 1;
		if (move[3*i+2] == WORDTRIS_EMPTY) 
			count--;
		else
			count++;
	}
	if (count + score < 1) died = 1;
	state.score = (pos->state ? ((Wordtris_state *)pos->state)->score : 0) + score;
	state.lives = (pos->state ? ((Wordtris_state *)pos->state)->lives : total_lives) - died;
	return &state;
}

int wordtris_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, 
		byte **movp, int ** rmovep)
{
	static int rmove[7];
	int *rp = rmove;
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	if (y != 0) return 0;
	if (wordtris_curx != x)
	{
		*rp++ = wordtris_curx;
		*rp++ = 0;
		*rp++ = 0;
		*rp++ = x;
		*rp++ = 0;
		*rp++ = RENDER_HIGHLIGHT1;
		*rp++ = -1;		
		*rmovep = rmove;
	}
	wordtris_curx = x;
	return 0;
}


gboolean wordtris_findletter (byte *board, int letter, int *x, int *y)
{
	int i, j;
	int minx = -1, miny = board_heit, minval = WORDTRIS_WILDCARD;
	for (i=0; i < WORDTRIS_LEN; i++)
	{
		for (j=1; j<board_heit; j++)
		{
			int val = board [j * board_wid + i];
			if (val == letter || val == WORDTRIS_WILDCARD)
			{
				if ((j < miny 
						&& (val != WORDTRIS_WILDCARD || minval == WORDTRIS_WILDCARD))
						|| (minval == WORDTRIS_WILDCARD && val != WORDTRIS_WILDCARD))
				{
					minx = i;
					miny = j;
					minval = val;
				}
			}
#ifdef ONLY_LOWEST
			else if (val != WORDTRIS_EMPTY)
				break;
#endif
		}
	}
	if (miny >= board_heit) return FALSE;
	*x = minx;
	*y = miny;
	return TRUE;
}

int wordtris_getmove_kb (Pos *pos, int key, byte **movp, int **rmovp)
{
	static int rmove[10];
	static byte move[10];
	byte *mp = move;
	int *rp = rmove;
	if (key == GDK_Right)
	{
		*rp++ = wordtris_curx;
		*rp++ = 0;
		*rp++ = 0;
		if (++wordtris_curx == WORDTRIS_LEN) wordtris_curx = 0;
		*rp++ = wordtris_curx;
		*rp++ = 0;
		*rp++ = RENDER_HIGHLIGHT1;
		*rp++ = -1;
		*rmovp = rmove;
		return 0;
	}
	if (key == GDK_Left)
	{
		*rp++ = wordtris_curx;
		*rp++ = 0;
		*rp++ = 0;
		if (--wordtris_curx < 0) wordtris_curx = WORDTRIS_LEN - 1;
		*rp++ = wordtris_curx;
		*rp++ = 0;
		*rp++ = RENDER_HIGHLIGHT1;
		*rp++ = -1;
		*rmovp = rmove;
		return 0;
	}
	if (key >= GDK_A && key <= GDK_Z)
		key = key + GDK_a - GDK_A;
	if (key >= GDK_a && key <= GDK_z)
	{
		int i, x, y;
		char word [WORDTRIS_LEN+1];
		gboolean found = wordtris_findletter (pos->board, key - GDK_a + 1, &x, &y);
		if (!found) return -1;
		if (wordtris_curx < 0) return -1;
		for (i=0; i<WORDTRIS_LEN; i++)
			word[i] = pos->board [i] - 1 + 'a';
		word [WORDTRIS_LEN] = '\0';
		if (word [wordtris_curx] == key - GDK_a + 'a') return -1;
		word [wordtris_curx] = key - GDK_a + 'a';
		if (!bsearch (word, flwords, num_flwords, sizeof (flwords[0]), wordtris_wordcmp))
			return -1;
		*mp++ = wordtris_curx; *mp++ = wordtris_cury; *mp++ = key - GDK_a + 1;
		*mp++ = x; *mp++ = y; *mp++ = WORDTRIS_EMPTY;
		*mp++ = -1;
		*movp = move;
		return 1;
	}
	return -1;
}

void wordtris_free ()
{
	wordtris_curx = wordtris_cury = 0;
}

int wordtris_get_rand_char ()
{
	int i, sum, thresh;
	if (random () % 10 == 0)
		return WORDTRIS_WILDCARD;
	for (i=0, sum=0; i<26; i++)
		sum += charcounts[i];
	thresh = random() % sum;
	for (i=0, sum=0; i<26; i++)
		if ((sum += charcounts[i]) > thresh)
			return i + 1;
	assert (0);
	return -1;
}

int wordtris_animate (Pos *pos, byte **movp)
{
	static byte move[1024];
	int i, j;
	byte *mp = move;
	byte *board = pos->board;
	for (i=0; i<board_wid; i++)
	for (j=board_heit-1; j>=1; j--)
	{
		int val;
		if ((val = pos->board [j * board_wid + i]) != WORDTRIS_EMPTY)
		{
			*mp++ = i;
			*mp++ = j;
			*mp++ = WORDTRIS_EMPTY;
			if (j > 2)
			{
				*mp++ = i;
				*mp++ = j-1;
				*mp++ = val;
			}
		}
	}
	while (1)
	{
		i = random() % WORDTRIS_LEN;
		if (pos->board [(board_heit - 1) * board_wid + i])
			continue;
		*mp++ = i;
		*mp++ = board_heit - 1;
		*mp++ = wordtris_get_rand_char();
		break;
	}
	*mp = -1;
	*movp = move;
	return 1;
}



