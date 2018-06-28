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
  <GND * 1 120 210 0 0 0 0>
  <GND * 1 130 430 0 0 0 0>
  <VProbe Pr1 1 610 250 28 -31 0 0>
  <GND * 1 620 270 0 0 0 0>
  <GND * 1 560 150 0 0 0 0>
  <VProbe Pr2 1 550 130 28 -31 0 0>
  <VProbe Pr3 1 390 310 28 -31 0 0>
  <GND * 1 400 330 0 0 0 0>
  <Vrect V1 1 120 180 18 -26 0 1 "1" 1 "1 ms" 1 "1 ms" 1 "1 ns" 0 "1 ns" 0 ".5m" 0>
  <Vrect V2 1 130 400 18 -26 0 1 "1" 1 "1 ms" 1 "1 ms" 1 "1 ns" 0 "1 ns" 0 ".01m" 0>
  <.TR TR1 1 440 560 0 185 0 0 "lin" 1 "0" 1 "10 ms" 1 "1000" 0 "Trapezoidal" 0 "2" 0 "1 ns" 0 "1e-16" 0 "150" 0 "0.001" 0 "1 pA" 0 "1 uV" 0 "26.85" 0 "1e-3" 0 "1e-6" 0 "1" 0 "CroutLU" 0 "no" 0 "yes" 0 "0" 0>
  <AND Y1 1 570 270 -26 27 0 0 "2" 0 "1 V" 0 "0" 0 "30" 0 "DIN40900" 0>
  <Inv Y3 1 580 470 -26 27 0 0 "1 V" 0 "0" 0 "10" 0 "old" 0>
  <NOR Y4 1 520 470 -26 27 0 0 "2" 0 "1 V" 0 "0" 0 "10" 0 "old" 0>
  <VProbe Pr4 1 620 450 28 -31 0 0>
  <GND * 1 630 470 0 0 0 0>
</Components>
<Wires>
  <120 150 460 150 "" 0 0 0 "">
  <540 150 540 260 "" 0 0 0 "">
  <540 280 540 370 "" 0 0 0 "">
  <130 370 380 370 "" 0 0 0 "">
  <380 370 490 370 "" 0 0 0 "">
  <380 330 380 370 "" 0 0 0 "">
  <490 370 540 370 "" 0 0 0 "">
  <460 150 540 150 "" 0 0 0 "">
  <490 370 490 460 "" 0 0 0 "">
  <460 150 460 480 "" 0 0 0 "">
  <460 480 490 480 "" 0 0 0 "">
</Wires>
<Diagrams>
  <Rect 985 690 1154 633 3 #c0c0c0 1 00 1 0 0.0005 0.01 1 -0.1 0.1 1.1 1 -1 0.2 1 315 0 225 "" "" "" "">
	<"Pr2.Vt" #0000ff 0 3 0 0 0>
	<"Pr3.Vt" #ff0000 0 3 0 0 0>
  </Rect>
  <Rect 835 1440 1154 633 3 #c0c0c0 1 00 1 0 0.0005 0.01 1 -0.1 0.1 1.1 1 -1 0.2 1 315 0 225 "" "" "" "">
	<"Pr1.Vt" #0000ff 0 3 0 0 0>
	<"Pr4.Vt" #ff0000 0 3 0 0 0>
  </Rect>
</Diagrams>
<Paintings>
</Paintings>
