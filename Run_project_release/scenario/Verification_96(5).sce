<?xml version="1.0" encoding="utf-8" standalone="yes"?>
<scenario>
  <name>Protocol&amp;Device</name>
  <Points>
    <item>
      <name>OpenProtocol</name>
      <receiver>run</receiver>
      <Parameters>
        <item>
          <name>FileName</name>
          <value>../scenario/Template_96(5)_Verification.trt</value>
        </item>
      </Parameters>
    </item>
    <item>
      <name>OpenDevice</name>
      <receiver>run</receiver>
    </item>
    <item>
      <name>ScanCoefficients</name>
      <receiver>run</receiver>
      <Parameters>
        <item>
          <name>TPeaks,Amplitude</name>
          <value></value>
        </item>
      </Parameters>
    </item>
    <item>
      <name>SetPlate</name>
      <receiver>run</receiver>
    </item>
    <item>
      <name>ChangeExposure</name>
      <receiver>run</receiver>
    </item>
    <item>
      <name>ChangeHeightTubes</name>
      <receiver>run</receiver>
    </item>
    <item>
      <name>RunProtocol</name>
      <receiver>run</receiver>
      <Parameters>
        <item>
          <name>VideoImage</name>
          <value>On</value>
        </item>
        <item>
          <name>Hold</name>
          <value>stop_after_execution</value>
        </item>
      </Parameters>
    </item>
    <item>
      <name>BackupHeightTubes</name>
      <receiver>run</receiver>
    </item>
    <item>
      <name>BackupExposure</name>
      <receiver>run</receiver>
    </item>
    <item>
      <name>AnalysisProtocol</name>
      <receiver>analysis</receiver>
      <Parameters>
        <item>
          <name>FileName</name>
          <value></value>
        </item>
      </Parameters>
    </item>
  </Points>
</scenario>