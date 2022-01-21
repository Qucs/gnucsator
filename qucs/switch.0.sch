<Qucs Schematic 0.0.20>
<Properties>
  <FrameText0=Title>
  <FrameText1=Drawn By:>
  <FrameText2=Date:>
  <FrameText3=Revision:>
  <OpenDisplay=1>
  <PosY=800>
  <RunScript=0>
  <showFrame=0>
</Properties>
<Symbol>
</Symbol>
<Components>
    <Vdc V1 1 80 90 18 -26 0 1 "1 V" 1>
    <GND * 5 80 120 0 0 0 0>
	 <.TR TR1 1 590 50 0 59 0 0 "lin" 1 "0" 1 "10 ms" 1 "11" 0 "Trapezoidal" 0 "2" 0 "1 ns" 0 "1e-16" 0 "150" 0 "0.001" 0 "1 pA" 0 "1 uV" 0 "26.85" 0 "1e-3" 0 "1e-6" 0 "1" 0 "CroutLU" 0 "no" 0 "yes" 0 "0" 0>
    <GND * 5 440 120 0 0 0 0>
    <C C1 1 440 90 17 -26 0 1 "100 nF" 1 "" 0 "neutral" 0>
    <R R1 1 350 60 -26 15 0 0 "1e6 Ohm" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
    <Switch S1 1 250 60 -26 11 0 0 "off" 1 "1 ms" 1 "0" 0 "1e12" 0 "26.85" 0 "1e-6" 0 "spline" 0>
    <Switch S2 1 530 60 -26 11 0 0 "on" 1 "1 ms" 1 "0" 0 "1e12" 0 "26.85" 0 "1e-6" 0 "spline" 0>
    <GND GND1 5 650 120 0 0 0 0>
</Components>
<Wires>
<80 60 220 60 "" 0 0 0 "">
<560 60 650 60 "" 0 0 0 "">
<650 120 650 60 "" 0 0 0 "">
<380 60 440 60 "chg" 440 20 20 "">
<500 60 500 0 "" 0 0 0 "">
<300 0 500 0 "" 0 0 0 "">
<320 60 300 60 "" 0 0 0 "">
<300 60 300 0 "" 0 0 0 "">
<280 60 320 60 "mid" 300 20 0 "">
</Wires>
<Diagrams>
  <Tab 70 463 298 263 1 #c0c0c0 1 00 1 0 1 1 1 0 1 1 1 0 1 11 0 0 0 "" "" "" "0">
	<"mid.Vt" #0000ff 0 5 0 0 0>
	<"chg.Vt" #0000ff 0 5 0 0 0>
  </Tab>
</Diagrams>
<Paintings>
</Paintings>
