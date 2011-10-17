#!MC 1100

# Use png output (otherwise avi)
$!VARSET |PNG|=0

$!VARSET |dir|='RESLT'

$!VARSET |nstep|=50
$!VARSET |lostep|=0
$!VARSET |dlstep|=1
$!LOOP |nstep|


$!DRAWGRAPHICS FALSE

$!NEWLAYOUT
$!PAPER SHOWPAPER = YES

$!VARSET |istep|=((|LOOP|-1)*|dlstep|+|lostep|)



$!READDATASET  '"|dir|/soln|istep|.dat" '
  READDATAOPTION = NEW
  RESETSTYLE = YES
  INCLUDETEXT = YES
  INCLUDEGEOM = NO
  INCLUDECUSTOMLABELS = NO
  VARLOADMODE = BYNAME
  ASSIGNSTRANDIDS = YES
  INITIALPLOTTYPE = CARTESIAN2D

#$!VIEW ZOOM
#   X1 = 3
#   Y1 = 3
#   X2 = 4
#   Y2 = 4          
$!GLOBALCONTOUR 1  VAR = 5
$!CONTOURLEVELS RESETTONICE
  CONTOURGROUP = 1
  APPROXNUMVALUES = 15
$!FIELDLAYERS SHOWCONTOUR = YES
$!FIELDLAYERS SHOWMESH = NO
$!FIELDMAP [1-|NUMZONES|]  EDGELAYER{LINETHICKNESS = 0.0200000000000000004}
$!GLOBALTWODVECTOR UVAR = 3
$!GLOBALTWODVECTOR VVAR = 4
$!RESETVECTORLENGTH 
$!GLOBALTWODVECTOR RELATIVELENGTH = 0.1
$!FIELDLAYERS SHOWVECTOR = YES
$!FIELDMAP [1-|NUMZONES|]  VECTOR{COLOR = WHITE}
$!TWODAXIS GRIDAREA{DRAWBORDER = YES}
$!TWODAXIS YDETAIL{TITLE{TITLEMODE = USETEXT}}
$!TWODAXIS YDETAIL{TITLE{TEXT = 'x<sub>2</sub>'}}
$!TWODAXIS XDETAIL{TITLE{TITLEMODE = USETEXT}}
$!TWODAXIS XDETAIL{TITLE{TEXT = 'x<sub>1</sub>'}}
$!FIELDMAP [1-|NUMZONES|]  VECTOR{COLOR = BLACK}
$!FIELDMAP [1-|NUMZONES|]  EDGELAYER{COLOR = PURPLE}






$!GLOBALCONTOUR 1  LEGEND{SHOW = YES}
#$!GLOBALCONTOUR 1  LEGEND{ISVERTICAL = NO}
#$!GLOBALCONTOUR 1  LABELS{AUTOLEVELSKIP = 4}
#$!GLOBALCONTOUR 1  LABELS{NUMFORMAT{FORMATTING = FIXEDFLOAT}}
#$!GLOBALCONTOUR 1  LABELS{NUMFORMAT{PRECISION = 2}}
#$!GLOBALCONTOUR 1  LABELS{NUMFORMAT{NEGATIVESUFFIX = ''}}
#$!GLOBALCONTOUR 1  LABELS{NUMFORMAT{ZEROSUFFIX = ''}}
#$!GLOBALCONTOUR 1  LEGEND{XYPOS{X = 85 Y=70}}






$!CONTOURLEVELS RESETTONICE
  CONTOURGROUP = 1
  APPROXNUMVALUES = 15
$!FIELDLAYERS SHOWVECTOR = YES

#$!GLOBALCONTOUR 1  LABELS{AUTOLEVELSKIP = 1}
#$!GLOBALCONTOUR 1  LABELS{NUMFORMAT{PRECISION = 5}}
#$!GLOBALCONTOUR 1  LABELS{NUMFORMAT{NEGATIVEPREFIX = ''}}
#$!GLOBALCONTOUR 1  LABELS{NUMFORMAT{ZEROPREFIX = ''}}
#$!GLOBALCONTOUR 1  LABELS{NUMFORMAT{PRECISION = 4}}


$!GLOBALCONTOUR 1  LEGEND{ISVERTICAL = NO}
$!GLOBALCONTOUR 1  LEGEND{BOX{MARGIN = 20}}
$!GLOBALCONTOUR 1  LEGEND{XYPOS{X = 75.211931818}}
$!GLOBALCONTOUR 1  LEGEND{XYPOS{Y = 66.797727273}}

$!CONTOURLEVELS NEW
  CONTOURGROUP = 1
  RAWDATA
12
0
0.5
1
1.5
2
2.5
3
3.5
4
4.5
5
5.5


$!DRAWGRAPHICS TRUE
$!REDRAWALL

$!IF |PNG|==1


        $!EXPORTSETUP EXPORTFORMAT = PNG
        $!EXPORTSETUP IMAGEWIDTH = 750
        $!EXPORTSETUP EXPORTFNAME = 'bubble|LOOP|.png'
        $!EXPORT
          EXPORTREGION = ALLFRAMES

$!ELSE

        $!IF |LOOP|>1
                $!EXPORTNEXTFRAME
        $!ELSE

                $!EXPORTSETUP
                  EXPORTREGION = ALLFRAMES
                  EXPORTFORMAT = AVI
                  EXPORTFNAME = "bubble.avi"
                $!EXPORTSETUP IMAGEWIDTH = 750
                $!EXPORTSTART
        $!ENDIF

$!ENDIF


$!ENDLOOP


$!IF |PNG|==0
        $!EXPORTFINISH
$!ENDIF
