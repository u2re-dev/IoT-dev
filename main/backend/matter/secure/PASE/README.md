# Passcode-Authenticated Session Establishment (PASE)

The Passcode-Authenticated Session Establishment (PASE) protocol aims to establish the **first session** between a Commissioner and a Commissionaire using a known passcode provided out-of-band. The pairing is performed using Section 3.10, "Password-Authenticated Key Exchange (PAKE)" and relies on a Password-Based Key Derivation Function (PBKDF) where the passcode is used as password.

This session establishment protocol provides a means to:

1. Communicate PBKDF parameters.
2. Derive PAKE bidirectional secrets.

The Commissioner is the Initiator and the Commissionaire is the Responder.

- Commissionaire - (session server, means app/server/tool/host/hub), initially (for/to) peer/device.
- Commissioner - (session client, means peer/device, require pass-code), initially (for/to) app/server/tool/host/hub.

I will not quoting `4.14. Protocol Details` from core spec.
