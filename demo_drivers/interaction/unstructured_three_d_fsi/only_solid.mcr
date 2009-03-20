#!MC 1100

$!VARSET |lostep|=0
$!VARSET |nstep|=9
$!VARSET |dstep|=1

$!VARSET |PNG|=0
$!VARSET |potential|=1


$!NEWLAYOUT 

$!IF |PNG|==0
     $!EXPORTSETUP EXPORTFORMAT = AVI
     $!EXPORTSETUP IMAGEWIDTH = 806
     $!EXPORTSETUP EXPORTFNAME = 'only_solid.avi'
     $!EXPORTSTART 
       EXPORTREGION = CURRENTFRAME
     $!EXPORTSETUP ANIMATIONSPEED = 1
$!ENDIF

$!LOOP |nstep|

$!VARSET |step|=(|lostep|+(|loop|-1)*|dstep|)

$!DRAWGRAPHICS FALSE


$!READDATASET  '"RESLT/solid_soln|step|.dat" '
  READDATAOPTION = NEW
  RESETSTYLE = YES
  INCLUDETEXT = NO
  INCLUDEGEOM = NO
  INCLUDECUSTOMLABELS = NO
  VARLOADMODE = BYNAME
  ASSIGNSTRANDIDS = YES
  INITIALPLOTTYPE = CARTESIAN3D

$!FIELDLAYERS SHOWMESH = NO
$!FIELDMAP [1-|NUMZONES|]  EDGELAYER{EDGETYPE = CREASES}
$!FIELDMAP [1-|NUMZONES|]  EDGELAYER{COLOR = RED}
$!FIELDMAP [1-|NUMZONES|]  EDGELAYER{LINETHICKNESS = 0.100000000000000006}
$!FIELDLAYERS SHOWSHADE = YES
$!REDRAWALL 
$!THREEDVIEW 
  PSIANGLE = 42.4211
  THETAANGLE = -124.212
  ALPHAANGLE = 120.444
  VIEWERPOSITION
    {
    X = 9.359687816579598
    Y = 6.363721613794445
    Z = 12.88578336948578
    }
$!VIEW PUSH
$!THREEDVIEW 
  PSIANGLE = 130.029
  THETAANGLE = -131.503
  ALPHAANGLE = 58.4958
  VIEWERPOSITION
    {
    X = 9.621736662872539
    Y = 8.513395975025668
    Z = -10.29132109800671
    }
$!VIEW PUSH
$!VIEW FIT

$!VIEW TRANSLATE
  X = 0.123954759863
  Y = 0
$!VIEW TRANSLATE
  X = 0.991638078902
  Y = 0
$!VIEW TRANSLATE
  X = 0.867683319039
  Y = 0
$!VIEW TRANSLATE
  X = 0.247909519726
  Y = 0
$!VIEW TRANSLATE
  X = 0.495819039451
  Y = 0
$!VIEW TRANSLATE
  X = 0.247909519726
  Y = 0
$!VIEW TRANSLATE
  X = 0.495819039451
  Y = 0
$!VIEW TRANSLATE
  X = 0.371864279588
  Y = 0
$!VIEW TRANSLATE
  X = 0.123954759863
  Y = 0
$!VIEW PUSH



$!DRAWGRAPHICS TRUE
$!REDRAWALL


$!IF |PNG|==1
     $!EXPORTSETUP EXPORTFORMAT = PNG
     $!EXPORTSETUP IMAGEWIDTH = 600
     $!EXPORTSETUP EXPORTFNAME = 'only_solid|loop|.png'
     $!EXPORT
       EXPORTREGION = ALLFRAMES
$!ELSE
     $!EXPORTNEXTFRAME
$!ENDIF

$!ENDLOOP

$!IF |PNG|==0
$!EXPORTFINISH
$!ENDIF
