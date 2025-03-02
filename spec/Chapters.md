TLV of ReadRequestMessage

```
[DEBUG] struct {
[DEBUG]   array m_0 = [
[DEBUG]     path {
[DEBUG]       uint8_t  m_2 = 0x00  #endpoint
[DEBUG]       uint8_t  m_3 = 0x30  #cluster
[DEBUG]       uint8_t  m_4 = 0x04  #attribute
[DEBUG]     }
[DEBUG]     path {
[DEBUG]       uint8_t  m_2 = 0x00  #endpoint
[DEBUG]       uint8_t  m_3 = 0x30  #cluster
[DEBUG]       uint8_t  m_4 = 0x00  #attribute
[DEBUG]     }
[DEBUG]     path {
[DEBUG]       uint8_t  m_2 = 0x00
[DEBUG]       uint8_t  m_3 = 0x30
[DEBUG]       uint8_t  m_4 = 0x01
[DEBUG]     }
[DEBUG]     path {
[DEBUG]       uint8_t  m_2 = 0x00
[DEBUG]       uint8_t  m_3 = 0x30
[DEBUG]       uint8_t  m_4 = 0x02
[DEBUG]     }
[DEBUG]     path {
[DEBUG]       uint8_t  m_2 = 0x00
[DEBUG]       uint8_t  m_3 = 0x30
[DEBUG]       uint8_t  m_4 = 0x03
[DEBUG]     }
[DEBUG]     path {
[DEBUG]       uint8_t  m_2 = 0x00
[DEBUG]       uint8_t  m_3 = 0x28
[DEBUG]       uint8_t  m_4 = 0x02
[DEBUG]     }
[DEBUG]     path {
[DEBUG]       uint8_t  m_2 = 0x00
[DEBUG]       uint8_t  m_3 = 0x28
[DEBUG]       uint8_t  m_4 = 0x04
[DEBUG]     }
[DEBUG]     path {
[DEBUG]       uint8_t  m_2 = 0x00    #endpoint
[DEBUG]       uint8_t  m_3 = 0x38    #cluster
[DEBUG]     }
[DEBUG]     path {
[DEBUG]       uint8_t  m_3 = 0x31    #cluster
 [DEBUG]      uint16_t m_4 = 0xfffc  #attribute (unknown)
[DEBUG]     }
[DEBUG]   ]
[DEBUG]   bool     m_3 = false
[DEBUG]   uint8_t  m_255 = 0x0c
[DEBUG] }
```

Chapters:

- 10.7.2.
- 10.6.2.
- 11.10.6 (Cluster 0x0030)
- 11.9.6  (Cluster 0x0031)
