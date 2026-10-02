# Raw predefined-frame capture

This diagnostic branch requests the documented Cyble `0x70` selectors **0 through
10**, once each. It retains the +10 dBm test setting and uses the reader’s saved
frequency. ATS is disabled; no meter configuration commands are sent.

Add under `everblu_meter:`:

```yaml
request_predefined_capture_button:
  name: "Capture Meter Predefined Frames"
stop_reading_button:
  name: "Stop Reading"
```

Start saving the ESPHome log at INFO level, then press **Capture Meter Predefined
Frames** during the meter’s listening window. Allow roughly a minute. Each selector
has one bounded attempt, with a pause between requests to let logs and API traffic
through. **Stop Reading** cancels before the next exchange. Unsupported selectors
may not reply; later selectors are still attempted.

Keep every `[CAPTURE]` line, from the initial notice through the final status.
Check that offsets are continuous; Wi-Fi log delivery can drop lines. A USB serial
log is preferable if convenient.
The log includes unencoded requests, complete decoded responses, oversampled RX
bytes, and separate length, address, control and CRC checks. Byte 16 is reported
without assigning a meaning across unverified response variants. Bytes beyond the
declared response length are labelled separately as trailing decode. Failed or
partial responses are retained too. The receive limit is 255 decoded bytes, matching
the RADIAN length byte; no assumed field layout shortens the capture.

**Keep these logs private.** They may contain the meter identity, consumption and
access code. Nothing is redacted from the capture. Do not commit real captures.

The button does not publish history or change stored tuning. Standard and Full FDR
reads remain separate. This capture button is exposed only through ESPHome.
