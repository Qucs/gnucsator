<Qucs Schematic >
<Properties>
  <View=0,0,2096,1436,1,0,0>
  <Grid=10,10,1>
  <DataSet=src2.dat>
  <DataDisplay=src2.dpl>
  <OpenDisplay=1>
  <Script=src2.m>
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
  <IProbe Pr1 1 780 260 -26 16 0 0>
  <.TR TR1 1 820 960 0 185 0 0 "lin" 1 "0" 1 "10 ms" 1 "1000" 0 "Trapezoidal" 0 "2" 0 "1 ns" 0 "1e-16" 0 "150" 0 "0.001" 0 "1 pA" 0 "1 uV" 0 "26.85" 0 "1e-3" 0 "1e-6" 0 "1" 0 "CroutLU" 0 "no" 0 "yes" 0 "0" 0>
  <Irect I1 1 560 460 24 -26 0 1 "1" 1 "3 ms" 1 "1 ms" 1 "1m" 0 ".5m" 0 "1.5m" 0>
  <IProbe Pr2 1 390 760 -26 16 0 0>
  <Ipulse I2 1 360 790 24 -26 0 1 "0" 1 "1 A" 1 "2m" 1 "8 ms" 1 "1 ms" 0 "2 ms" 0>
</Components>
<Wires>
  <560 260 560 430 "" 0 0 0 "">
  <560 260 750 260 "" 0 0 0 "">
  <810 260 810 490 "" 0 0 0 "">
  <560 490 810 490 "" 0 0 0 "">
  <420 760 420 820 "" 0 0 0 "">
  <360 820 420 820 "" 0 0 0 "">
</Wires>
<Diagrams>
  <Rect 895 780 1154 633 3 #c0c0c0 1 00 1 0 0.0005 0.01 1 -0.1 0.1 1.1 1 -1 0.2 1 315 0 225 "" "" "" "">
	<"Pr1.It" #0000ff 0 3 0 0 0>
	<"Pr2.It" #ff0000 0 3 0 0 0>
  </Rect>
</Diagrams>
<Paintings>
</Paintings>
