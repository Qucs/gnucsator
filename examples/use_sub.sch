<Qucs Schematic >
<Properties>
  <View=0,-120,1220,1229,1.77156,0,89>
  <Grid=10,10,1>
  <DataSet=use_sub.dat>
  <DataDisplay=use_sub.dpl>
  <OpenDisplay=1>
  <Script=use_sub.m>
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
  <GND * 1 160 250 0 0 0 0>
  <GND * 1 490 180 0 0 0 0>
  <VProbe Pr1 1 440 100 28 -31 0 0>
  <.TR TR1 1 250 400 0 185 0 0 "lin" 1 "0" 1 "1" 1 "11" 0 "Trapezoidal" 0 "2" 0 "1 ns" 0 "1e-16" 0 "150" 0 "0.001" 0 "1 pA" 0 "1 uV" 0 "26.85" 0 "1e-3" 0 "1e-6" 0 "1" 0 "CroutLU" 0 "no" 0 "yes" 0 "0" 0>
  <Vac V1 1 160 220 18 -26 0 1 "1 V" 1 "1" 0 "0" 0 "0" 0>
  <R R1 1 460 180 -26 15 0 0 "100 Ohm" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <VProbe Pr2 1 430 -40 28 -31 0 0>
  <GND * 1 440 -20 0 0 0 0>
  <Sub foo1 1 270 -40 -26 21 0 0 "mysub.sch" 0>
  <Sub foo 1 330 180 -26 21 0 0 "mysub.sch" 0>
</Components>
<Wires>
  <360 180 430 180 "" 0 0 0 "">
  <430 120 430 180 "" 0 0 0 "">
  <450 120 490 120 "" 0 0 0 "">
  <490 120 490 180 "" 0 0 0 "">
  <300 180 300 190 "" 0 0 0 "">
  <160 190 300 190 "" 0 0 0 "">
  <160 -40 240 -40 "" 0 0 0 "">
  <160 -40 160 190 "" 0 0 0 "">
  <300 -40 300 -20 "" 0 0 0 "">
  <300 -20 420 -20 "" 0 0 0 "">
  <360 180 360 180 "post" 390 150 0 "">
  <160 190 160 190 "pre" 190 160 0 "">
</Wires>
<Diagrams>
  <Rect 940 330 240 160 3 #c0c0c0 1 00 1 0 0.2 1 1 -1 1 1 1 -1 1 1 315 0 225 "" "" "" "">
	<"Pr1.Vt" #0000ff 0 3 0 0 0>
	<"Pr2.Vt" #ff0000 0 3 0 0 0>
  </Rect>
</Diagrams>
<Paintings>
</Paintings>
