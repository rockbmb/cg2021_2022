#!/usr/bin/zsh
MERCURY_R=30
VENUS_R=$(python -c "print( $MERCURY_R * 1.5 )")
EARTH_R=$(python -c "print( $MERCURY_R * 1.55 )")
MARS_R=$(python -c "print( $MERCURY_R * 1.4 )")
JUPITER_R=$(python -c "print( $MERCURY_R * 2.8 )")
SATURN_R=$(python -c "print( $MERCURY_R * 2 )")
URANUS_R=$(python -c "print( $MERCURY_R * 1.9 )")
NEPTUNE_R=$(python -c "print( $MERCURY_R * 1.5 )")
SUN_R=$(python -c "print( $MERCURY_R * 3.3 )")

RES=64

../bin/generator sphere $MERCURY_R $RES $RES mercury.3d
../bin/generator sphere "$VENUS_R" $RES $RES venus.3d
../bin/generator sphere "$EARTH_R" $RES $RES earth.3d
../bin/generator sphere "$MARS_R" $RES $RES mars.3d
../bin/generator sphere "$JUPITER_R" $RES $RES jupiter.3d
../bin/generator sphere "$SATURN_R" $RES $RES saturn.3d
../bin/generator sphere "$URANUS_R" $RES $RES uranus.3d
../bin/generator sphere "$NEPTUNE_R" $RES $RES neptune.3d
../bin/generator sphere "$SUN_R" $RES $RES sun.3d
../bin/generator bezier ../test_files_phase_3/teapot.patch 10 teapot.3d

MERCURY_D=$(((SUN_R+MERCURY_R)*1.5))
VENUS_D=$(((MERCURY_D+VENUS_R)*2.5))
EARTH_D=$(((VENUS_D+EARTH_R)*3.5))
MARS_D=$(((EARTH_D+MARS_R)*4.5))
JUPITER_D=$(((MARS_D+JUPITER_R)*5.5))
SATURN_D=$(((JUPITER_D+SATURN_R)*6.5))
URANUS_D=$(((SATURN_D+URANUS_R)*7.5))
NEPTUNE_D=$(((URANUS_D+NEPTUNE_R)*8.5))

sed -e "s/MERCURY/$MERCURY_D/g"\
    -e "s/VENUS/$VENUS_D/g"\
    -e "s/EARTH/$EARTH_D/g"\
    -e "s/MARS/$MARS_D/g"\
    -e "s/JUPITER/$JUPITER_D/g"\
    -e "s/SATURN/$SATURN_D/g"\
    -e "s/URANUS/$URANUS_D/g"\
    -e "s/NEPTUNE/$NEPTUNE_D/g"\
    solar_system.xml.template > solar_system.xml

../bin/engine solar_system.xml
rm mercury.3d venus.3d earth.3d mars.3d jupiter.3d saturn.3d uranus.3d neptune.3d sun.3d solar_system.xml