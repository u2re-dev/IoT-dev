# Certificate Authenticated Session Establishment (CASE)

НЕ путать с `CASE (англ. computer-aided software engineering)`.

This **session establishment protocol** provides a means to:

1. Mutually authenticate both peer Nodes
2. Generate cryptographic keys to secure subsequent communication within a session
3. Exchange operational parameters for the session, such as Session Identifier and MRP parameters

Contains three stage (sigma1, sigma2, sigma3) to generate keys.

1. Sigma-1 from initator (non-encrypted).
2. Sigma-2 from responder (encrypted).
3. Sigma-3 from initator (encrypted, answer).
4. Final stage (verify) in responder.

Resumption has only sigma-1, sigma-2 and finish stage.
