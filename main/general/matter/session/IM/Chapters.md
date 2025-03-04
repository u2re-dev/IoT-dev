TLV of ReadRequestMessage (0x02)

```
[DEBUG] struct {
[DEBUG]   array m_0 = [ # Attribut­eRequests
[DEBUG]     path {
[DEBUG]       uint8_t  m_2 = 0x00    #endpoint
[DEBUG]       uint8_t  m_3 = 0x30    #cluster   (General Commissioning Cluster)
[DEBUG]       uint8_t  m_4 = 0x04    #attribute (SupportsConcurrentConnection)
[DEBUG]     }
[DEBUG]     path {
[DEBUG]       uint8_t  m_2 = 0x00    #endpoint
[DEBUG]       uint8_t  m_3 = 0x30    #cluster   (General Commissioning Cluster)
[DEBUG]       uint8_t  m_4 = 0x00    #attribute (Breadcrumb)
[DEBUG]     }
[DEBUG]     path {
[DEBUG]       uint8_t  m_2 = 0x00
[DEBUG]       uint8_t  m_3 = 0x30    # ... (General Commissioning Cluster)
[DEBUG]       uint8_t  m_4 = 0x01    # ... (BasicCommissioningInfo)
[DEBUG]     }
[DEBUG]     path {
[DEBUG]       uint8_t  m_2 = 0x00
[DEBUG]       uint8_t  m_3 = 0x30    # ... (General Commissioning Cluster)
[DEBUG]       uint8_t  m_4 = 0x02    # ... (RegulatoryConfig)
[DEBUG]     }
[DEBUG]     path {
[DEBUG]       uint8_t  m_2 = 0x00    #
[DEBUG]       uint8_t  m_3 = 0x30    # ... (General Commissioning Cluster)
[DEBUG]       uint8_t  m_4 = 0x03    # ... (LocationCapability)
[DEBUG]     }
[DEBUG]     path {
[DEBUG]       uint8_t  m_2 = 0x00    #
[DEBUG]       uint8_t  m_3 = 0x28    # ... (Basic Information)
[DEBUG]       uint8_t  m_4 = 0x02    # ... (VendorID)
[DEBUG]     }
[DEBUG]     path {
[DEBUG]       uint8_t  m_2 = 0x00
[DEBUG]       uint8_t  m_3 = 0x28    # ... (Basic Information)
[DEBUG]       uint8_t  m_4 = 0x04    # ... (ProductID)
[DEBUG]     }
[DEBUG]     path {
[DEBUG]       uint8_t  m_2 = 0x00    #endpoint
[DEBUG]       uint8_t  m_3 = 0x38    #cluster  (Time Synchronization Cluster)
[DEBUG]     }
[DEBUG]     path {
[DEBUG]       uint8_t  m_3 = 0x31    #cluster   (Network Commissioning Cluster)
[DEBUG]       uint16_t m_4 = 0xfffc  #attribute (unknown)
[DEBUG]     }
[DEBUG]   ]
[DEBUG]   bool     m_3 = false  # FabricFiltered
[DEBUG]   uint8_t  m_255 = 0x0c
[DEBUG] }
```

Chapters:

- 10.7.2.
- 10.6.2.
- 11.10.6 (Cluster 0x0030)
- 11.9.6  (Cluster 0x0031)
- 11.1.5

```
[1740914101.916] [75591:75619] [EM] <<5 [E:22519i S:8006 M:112589859] (S) Msg Retransmission to 0:FFFFFFFB00000000 failure (max retries:4)
[1740914125.626] [75591:75619] [DMG] Time out! failed to receive report data from Exchange: 22519i
[1740914125.626] [75591:75619] [CTL] Failed to read BasicCommissioningInfo: src/app/ClusterStateCache.cpp:336: CHIP Error 0x00000010: Key not found
[1740914125.626] [75591:75619] [CTL] Failed to read RegulatoryConfig: src/app/ClusterStateCache.cpp:336: CHIP Error 0x00000010: Key not found
[1740914125.626] [75591:75619] [CTL] Failed to read LocationCapability: src/app/ClusterStateCache.cpp:336: CHIP Error 0x00000010: Key not found
[1740914125.626] [75591:75619] [CTL] Failed to read Breadcrumb: src/app/ClusterStateCache.cpp:336: CHIP Error 0x00000010: Key not found
[1740914125.626] [75591:75619] [CTL] Ignoring failure to read SupportsConcurrentConnection: src/app/ClusterStateCache.cpp:336: CHIP Error 0x00000010: Key not found
[1740914125.626] [75591:75619] [CTL] Failed to read VendorID: src/app/ClusterStateCache.cpp:336: CHIP Error 0x00000010: Key not found
[1740914125.626] [75591:75619] [CTL] Failed to read ProductID: src/app/ClusterStateCache.cpp:336: CHIP Error 0x00000010: Key not found
[1740914125.626] [75591:75619] [CTL] Error on commissioning step 'ReadCommissioningInfo': 'src/app/ClusterStateCache.cpp:336: CHIP Error 0x00000010: Key not found'
```
