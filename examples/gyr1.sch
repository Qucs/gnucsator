<Qucs Schematic >
<Properties>
  <View=0,0,2179,1687,1,0,0>
  <Grid=10,10,1>
  <DataSet=logic1.dat>
  <DataDisplay=logic1.dpl>
  <OpenDisplay=1>
  <Script=logic1.m>
  <RunScript=0>
  <showFrame=0>
  <FrameText0=Title>
  <FrameText1=Drawn By:>
  <FrameText2=Date:>
  <FrameText3=Revision:>
</Properties>
<Symbol>
</Symbol>
<Components>
  <GND * 1 130 430 0 0 0 0>
  <VProbe Pr3 1 390 310 28 -31 0 0>
  <GND * 1 400 330 0 0 0 0>
  <Vrect V2 1 130 400 18 -26 0 1 "1" 1 "1 ms" 1 "1 ms" 1 "1 ns" 0 "1 ns" 0 ".01m" 0>
  <.TR TR1 1 440 560 0 185 0 0 "lin" 1 "0" 1 "10 ms" 1 "1000" 0 "Trapezoidal" 0 "2" 0 "1 ns" 0 "1e-16" 0 "150" 0 "0.001" 0 "1 pA" 0 "1 uV" 0 "26.85" 0 "1e-3" 0 "1e-6" 0 "1" 0 "CroutLU" 0 "no" 0 "yes" 0 "0" 0>
  <R R1 1 350 370 -26 15 0 0 "50 Ohm" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <Gyrator X1 1 620 400 -26 34 0 0 "50 Ohm" 1 "50 Ohm" 0>
  <GND * 1 590 430 0 0 0 0>
  <GND * 1 650 430 0 0 0 0>
  <Vdc V3 1 1030 410 18 -26 0 1 "1 V" 1>
  <GND * 1 1030 440 0 0 0 0>
  <R R2 1 1000 380 -26 15 0 0 "50 Ohm" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <VProbe Pr4 1 810 310 28 -31 0 0>
  <GND * 1 820 330 0 0 0 0>
</Components>
<Wires>
  <380 370 590 370 "" 0 0 0 "">
  <380 330 380 370 "" 0 0 0 "">
  <130 370 320 370 "" 0 0 0 "">
  <970 370 970 380 "" 0 0 0 "">
  <650 370 800 370 "" 0 0 0 "">
  <800 370 970 370 "" 0 0 0 "">
  <800 330 800 370 "" 0 0 0 "">
</Wires>
<Diagrams>
  <Rect 835 1440 1154 633 3 #c0c0c0 1 00 1 0 0.0005 0.01 1 -0.65 0.2 1.2 1 -1 0.2 1 315 0 225 "" "" "" "">
	<"Pr1.Vt" #0000ff 0 3 0 0 0>
	<"Pr3.Vt" #ff0000 0 3 0 0 0>
	<"Pr4.Vt" #ff00ff 0 3 0 0 0>
  </Rect>
</Diagrams>
<Paintings>
</Paintings>
