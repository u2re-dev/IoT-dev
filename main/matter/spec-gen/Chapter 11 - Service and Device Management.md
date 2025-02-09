# Chapter 11 - Service and Device Management

## Основные положения

* **Кластер Basic Information:** Предоставляет атрибуты и события для определения основной информации об узлах (Nodes), такой как Vendor ID, Product ID и серийный номер.
* **Кластер Group Key Management:** Управляет групповыми ключами для узла, поддерживая список групп и их членство.
* **Кластер Localization Configuration:** Поддерживает интерфейс для определения и настройки информации о локализации (язык, единицы измерения) для узла.
* **Кластер Time Format Localization:** Предоставляет атрибуты для настройки формата времени и даты.
* **Кластер Unit Localization:** Позволяет настраивать единицы измерения, используемые для отображения значений.
* **Кластер Power Source Configuration:** Описывает конфигурацию и возможности системы питания устройства, связывая с одним или несколькими кластерами Power Source.
* **Кластер Power Source:** Описывает конфигурацию и возможности физического источника питания.
* **Кластер Power Topology:** Предоставляет механизм для выражения потока мощности между конечными точками (endpoints).
* **Кластер Network Commissioning:** Связывает узел с одной или несколькими сетевыми интерфейсами (Wi-Fi, Ethernet, Thread).
* **Кластер General Commissioning:** Управляет базовым жизненным циклом ввода в эксплуатацию (commissioning) и содержит функции, не относящиеся к другим кластерам.
* **Кластер Diagnostic Logs:** Предоставляет команды для получения неструктурированных диагностических журналов с узла.
* **Кластер General Diagnostics:** Предоставляет стандартизированные метрики диагностики, релевантные для большинства узлов.
* **Кластер Software Diagnostics:** Предоставляет метрики, относящиеся к программному обеспечению, работающему на узле.
* **Кластер Thread Network Diagnostics:** Предоставляет метрики, относящиеся к сети Thread.
* **Кластер Wi-Fi Network Diagnostics:** Предоставляет метрики, относящиеся к сети Wi-Fi.
* **Кластер Ethernet Network Diagnostics:** Предоставляет метрики, относящиеся к сети Ethernet.
* **Кластер Time Synchronization:** Обеспечивает механизм для синхронизации времени узлов.
* **Кластер Node Operational Credentials:** Управляет Node Operational credentials на Commissionee или Node, а также управляет связанными Fabrics.
* **Кластер Administrator Commissioning:** Используется для запуска узла, чтобы разрешить ввод в эксплуатацию новому администратору.
* **Кластер Diagnostic Logs:** Предоставляет команды для получения неструктурированных диагностических журналов с узла.
* **Кластер OTA Software Update:** Поддерживает Over-the-Air (OTA) обновление программного обеспечения.
* **Кластер Joint Fabric Datastore:** Предоставляет механизм для администраторов Joint Fabric для управления набором узлов, групп и членством в группах среди узлов в Joint Fabric.
* **Кластер Joint Fabric PKI:** Предоставляет механизм для управления PKI в Joint Fabric.
* **Кластер Commissioner Control:** Поддерживает возможность для клиентов запрашивать ввод в эксплуатацию себя или других узлов в fabric, в который может быть введен в эксплуатацию кластерный сервер.

### По описаниям

Вот структуры C/C++ и их краткое описание, найденные в предоставленном тексте:

1. **ProductFinishEnum Type**: Перечисление, представляющее видимую отделку продукта (например, матовую, глянцевую).
2. **ColorEnum Type**: Перечисление, представляющее цвет продукта (например, черный, синий, красный).
3. **ProductAppearanceStruct Type**: Структура, содержащая описание внешнего вида продукта, включая отделку и основной цвет.
4. **CapabilityMinimaStruct Type**: Структура, содержащая минимальные значения для общих возможностей узла, таких как количество одновременных сеансов CASE и подписок на структуру.
5. **GroupKeySecurityPolicyEnum Type**: Перечисление, представляющее политику безопасности для ключей группы (например, trust-first, cache-and-sync).
6. **GroupKeyMulticastPolicyEnum Type**: Перечисление, представляющее политику многоадресной рассылки для ключей группы (например, PerGroupID, AllNodes).
7. **GroupKeyMapStruct Type**: Структура, связывающая идентификатор группы с набором ключей группы.
8. **GroupKeySetStruct Type**: Структура, содержащая набор ключей группы, политику безопасности и политику многоадресной рассылки.
9. **GroupInfoMapStruct Type**: Структура, содержащая информацию о группе, такую как идентификатор группы, список конечных точек и имя группы.
10. **HourFormatEnum Type**: Перечисление, представляющее формат времени (например, 12-часовой, 24-часовой).
11. **CalendarTypeEnum Type**: Перечисление, представляющее тип календаря (например, буддийский, китайский, григорианский).
12. **TempUnitEnum Type**: Перечисление, представляющее единицу измерения температуры (например, Фаренгейт, Цельсий, Кельвин).
13. **WiredFaultEnum Type**: Перечисление, представляющее неисправности проводного источника питания (например, перенапряжение, пониженное напряжение).
14. **BatFaultEnum Type**: Перечисление, представляющее неисправности батареи (например, перегрев, недогрев).
15. **BatChargeFaultEnum Type**: Перечисление, представляющее неисправности зарядки батареи (например, слишком высокая температура окружающей среды, отсутствие батареи).
16. **PowerSourceStatusEnum Type**: Перечисление, представляющее статус источника питания (например, активный, в режиме ожидания, недоступный).
17. **WiredCurrentTypeEnum Type**: Перечисление, представляющее тип проводного тока (например, переменный ток, постоянный ток).
18. **BatChargeLevelEnum Type**: Перечисление, представляющее уровень заряда батареи (например, ОК, предупреждение, критический).
19. **BatReplaceabilityEnum Type**: Перечисление, представляющее возможность замены батареи (например, не подлежит замене, заменяется пользователем, заменяется на заводе).
20. **BatCommonDesignationEnum Type**: Перечисление, представляющее общее обозначение батареи (например, AAA, AA, C).
21. **BatApprovedChemistryEnum Type**: Перечисление, представляющее утвержденный химический состав батареи (например, щелочной, литий-ионный).
22. **BatChargeStateEnum Type**: Перечисление, представляющее состояние заряда батареи (например, неизвестно, заряжается, полностью заряжен).
23. **WiFiSecurityBitmap Type**: Битовая карта, представляющая поддерживаемые типы безопасности Wi-Fi (например, незашифрованный, WEP, WPA).
24. **ThreadCapabilitiesBitmap Type**: Битовая карта, представляющая поддерживаемые возможности Thread (например, возможность Border Router, возможность Router).
25. **WiFiBandEnum Type**: Перечисление, представляющее поддерживаемые диапазоны Wi-Fi (например, 2,4 ГГц, 5 ГГц).
26. **NetworkCommissioningStatusEnum Type**: Перечисление, представляющее статус сетевой комиссии (например, успех, вне диапазона, не найден сетевой идентификатор).
27. **NetworkInfoStruct Type**: Структура, содержащая информацию о сети, такую как сетевой идентификатор и статус подключения.
28. **WiFiInterfaceScanResultStruct Type**: Структура, содержащая результаты сканирования интерфейса Wi-Fi, такие как безопасность, SSID и канал.
29. **ThreadInterfaceScanResultStruct Type**: Структура, содержащая результаты сканирования сети Thread, такие как PanID, ExtendedPanId и канал.
30. **CommissioningErrorEnum Type**: Перечисление, представляющее ошибки комиссии (например, ОК, значение вне диапазона, недопустимая аутентификация).
31. **RegulatoryLocationTypeEnum Type**: Перечисление, представляющее тип местоположения регулирования (например, в помещении, на открытом воздухе, в помещении/на открытом воздухе).
32. **BasicCommissioningInfo Type**: Структура, содержащая основную информацию о комиссии, такую как срок действия отказоустойчивости и максимальное количество отказоустойчивых секунд.
33. **HardwareFaultEnum Type**: Перечисление, представляющее неисправности оборудования (например, не указано, радио, датчик).
34. **RadioFaultEnum Type**: Перечисление, представляющее неисправности радиосвязи (например, не указано, Wi-Fi, сотовая связь).
35. **NetworkFaultEnum Type**: Перечисление, представляющее неисправности сети (например, не указано, сбой оборудования, сеть забита).
36. **InterfaceTypeEnum Type**: Перечисление, представляющее тип интерфейса (например, не указано, Wi-Fi, Ethernet).
37. **BootReasonEnum Type**: Перечисление, представляющее причину загрузки (например, не указано, перезагрузка при включении питания, сброс при просадке напряжения).
38. **NetworkInterface Type**: Структура, содержащая информацию о сетевом интерфейсе, такую как имя, статус и адреса.
39. **TempUnitEnum Type**: Перечисление, представляющее единицу измерения температуры (например, Фаренгейт, Цельсий, Кельвин).
40. **ThreadMetricsStruct Type**: Структура, содержащая метрики Thread, такие как ID, имя и свободная память стека.
41. **WiredFaultEnum Type**: Перечисление, представляющее неисправности проводного источника питания (например, перенапряжение, пониженное напряжение).
42. **PowerSourceStatusEnum Type**: Перечисление, представляющее статус источника питания (например, активный, в режиме ожидания, недоступный).
43. **WiredCurrentTypeEnum Type**: Перечисление, представляющее тип проводного тока (например, переменный ток, постоянный ток).
44. **BatChargeLevelEnum Type**: Перечисление, представляющее уровень заряда батареи (например, ОК, предупреждение, критический).
45. **BatReplaceabilityEnum Type**: Перечисление, представляющее возможность замены батареи (например, не подлежит замене, заменяется пользователем, заменяется на заводе).
46. **BatCommonDesignationEnum Type**: Перечисление, представляющее общее обозначение батареи (например, AAA, AA, C).
47. **BatApprovedChemistryEnum Type**: Перечисление, представляющее утвержденный химический состав батареи (например, щелочной, литий-ионный).
48. **BatChargeStateEnum Type**: Перечисление, представляющее состояние заряда батареи (например, неизвестно, заряжается, полностью заряжен).
49. **PowerSourceStatusEnum Type**: Перечисление, представляющее статус источника питания (например, активный, в режиме ожидания, недоступный).
50. **WiredCurrentTypeEnum Type**: Перечисление, представляющее тип проводного тока (например, переменный ток, постоянный ток).
51. **BatChargeLevelEnum Type**: Перечисление, представляющее уровень заряда батареи (например, ОК, предупреждение, критический).
52. **BatReplaceabilityEnum Type**: Перечисление, представляющее возможность замены батареи (например, не подлежит замене, заменяется пользователем, заменяется на заводе).
53. **BatCommonDesignationEnum Type**: Перечисление, представляющее общее обозначение батареи (например, AAA, AA, C).
54. **BatApprovedChemistryEnum Type**: Перечисление, представляющее утвержденный химический состав батареи (например, щелочной, литий-ионный).
55. **BatChargeStateEnum Type**: Перечисление, представляющее состояние заряда батареи (например, неизвестно, заряжается, полностью заряжен).
56. **PowerSourceStatusEnum Type**: Перечисление, представляющее статус источника питания (например, активный, в режиме ожидания, недоступный).
57. **WiredCurrentTypeEnum Type**: Перечисление, представляющее тип проводного тока (например, переменный ток, постоянный ток).
58. **BatChargeLevelEnum Type**: Перечисление, представляющее уровень заряда батареи (например, ОК, предупреждение, критический).
59. **BatReplaceabilityEnum Type**: Перечисление, представляющее возможность замены батареи (например, не подлежит замене, заменяется пользователем, заменяется на заводе).
60. **BatCommonDesignationEnum Type**: Перечисление, представляющее общее обозначение батареи (например, AAA, AA, C).
61. **BatApprovedChemistryEnum Type**: Перечисление, представляющее утвержденный химический состав батареи (например, щелочной, литий-ионный).
62. **BatChargeStateEnum Type**: Перечисление, представляющее состояние заряда батареи (например, неизвестно, заряжается, полностью заряжен).
63. **PowerSourceStatusEnum Type**: Перечисление, представляющее статус источника питания (например, активный, в режиме ожидания, недоступный).
64. **WiredCurrentTypeEnum Type**: Перечисление, представляющее тип проводного тока (например, переменный ток, постоянный ток).
65. **BatChargeLevelEnum Type**: Перечисление, представляющее уровень заряда батареи (например, ОК, предупреждение, критический).
66. **BatReplaceabilityEnum Type**: Перечисление, представляющее возможность замены батареи (например, не подлежит замене, заменяется пользователем, заменяется на заводе).
67. **BatCommonDesignationEnum Type**: Перечисление, представляющее общее обозначение батареи (например, AAA, AA, C).
68. **BatApprovedChemistryEnum Type**: Перечисление, представляющее утвержденный химический состав батареи (например, щелочной, литий-ионный).
69. **BatChargeStateEnum Type**: Перечисление, представляющее состояние заряда батареи (например, неизвестно, заряжается, полностью заряжен).
70. **PowerSourceStatusEnum Type**: Перечисление, представляющее статус источника питания (например, активный, в режиме ожидания, недоступный).
71. **WiredCurrentTypeEnum Type**: Перечисление, представляющее тип проводного тока (например, переменный ток, постоянный ток).
72. **BatChargeLevelEnum Type**: Перечисление, представляющее уровень заряда батареи (например, ОК, предупреждение, критический).
73. **BatReplaceabilityEnum Type**: Перечисление, представляющее возможность замены батареи (например, не подлежит замене, заменяется пользователем, заменяется на заводе).
74. **BatCommonDesignationEnum Type**: Перечисление, представляющее общее обозначение батареи (например, AAA, AA, C).
75. **BatApprovedChemistryEnum Type**: Перечисление, представляющее утвержденный химический состав батареи (например, щелочной, литий-ионный).
76. **BatChargeStateEnum Type**: Перечисление, представляющее состояние заряда батареи (например, неизвестно, заряжается, полностью заряжен).
77. **PowerSourceStatusEnum Type**: Перечисление, представляющее статус источника питания (например, активный, в режиме ожидания, недоступный).
78. **WiredCurrentTypeEnum Type**: Перечисление, представляющее тип проводного тока (например, переменный ток, постоянный ток).
79. **BatChargeLevelEnum Type**: Перечисление, представляющее уровень заряда батареи (например, ОК, предупреждение, критический).
80. **BatReplaceabilityEnum Type**: Перечисление, представляющее возможность замены батареи (например, не подлежит замене, заменяется пользователем, заменяется на заводе).
81. **BatCommonDesignationEnum Type**: Перечисление, представляющее общее обозначение батареи (например, AAA, AA, C).
82. **BatApprovedChemistryEnum Type**: Перечисление, представляющее утвержденный химический состав батареи (например, щелочной, литий-ионный).
83. **BatChargeStateEnum Type**: Перечисление, представляющее состояние заряда батареи (например, неизвестно, заряжается, полностью заряжен).
84. **PowerSourceStatusEnum Type**: Перечисление, представляющее статус источника питания (например, активный, в режиме ожидания, недоступный).
85. **WiredCurrentTypeEnum Type**: Перечисление, представляющее тип проводного тока (например, переменный ток, постоянный ток).
86. **BatChargeLevelEnum Type**: Перечисление, представляющее уровень заряда батареи (например, ОК, предупреждение, критический).
87. **BatReplaceabilityEnum Type**: Перечисление, представляющее возможность замены батареи (например, не подлежит замене, заменяется пользователем, заменяется на заводе).
88. **BatCommonDesignationEnum Type**: Перечисление, представляющее общее обозначение батареи (например, AAA, AA, C).
89. **BatApprovedChemistryEnum Type**: Перечисление, представляющее утвержденный химический состав батареи (например, щелочной, литий-ионный).
90. **BatChargeStateEnum Type**: Перечисление, представляющее состояние заряда батареи (например, неизвестно, заряжается, полностью заряжен).
91. **PowerSourceStatusEnum Type**: Перечисление, представляющее статус источника питания (например, активный, в режиме ожидания, недоступный).
92. **WiredCurrentTypeEnum Type**: Перечисление, представляющее тип проводного тока (например, переменный ток, постоянный ток).
93. **BatChargeLevelEnum Type**: Перечисление, представляющее уровень заряда батареи (например, ОК, предупреждение, критический).
94. **BatReplaceabilityEnum Type**: Перечисление, представляющее возможность замены батареи (например, не подлежит замене, заменяется пользователем, заменяется на заводе).
95. **BatCommonDesignationEnum Type**: Перечисление, представляющее общее обозначение батареи (например, AAA, AA, C).
96. **BatApprovedChemistryEnum Type**: Перечисление, представляющее утвержденный химический состав батареи (например, щелочной, литий-ионный).
97. **BatChargeStateEnum Type**: Перечисление, представляющее состояние заряда батареи (например, неизвестно, заряжается, полностью заряжен).
98. **PowerSourceStatusEnum Type**: Перечисление, представляющее статус источника питания (например, активный, в режиме ожидания, недоступный).
99. **WiredCurrentTypeEnum Type**: Перечисление, представляющее тип проводного тока (например, переменный ток, постоянный ток).
100. **BatChargeLevelEnum Type**: Перечисление, представляющее уровень заряда батареи (например, ОК, предупреждение, критический).
101. **BatReplaceabilityEnum Type**: Перечисление, представляющее возможность замены батареи (например, не подлежит замене, заменяется пользователем, заменяется на заводе).
102. **BatCommonDesignationEnum Type**: Перечисление, представляющее общее обозначение батареи (например, AAA, AA, C).
103. **BatApprovedChemistryEnum Type**: Перечисление, представляющее утвержденный химический состав батареи (например, щелочной, литий-ионный).
104. **BatChargeStateEnum Type**: Перечисление, представляющее состояние заряда батареи (например, неизвестно, заряжается, полностью заряжен).
105. **PowerSourceStatusEnum Type**: Перечисление, представляющее статус источника питания (например, активный, в режиме ожидания, недоступный).
106. **WiredCurrentTypeEnum Type**: Перечисление, представляющее тип проводного тока (например, переменный ток, постоянный ток).
107. **BatChargeLevelEnum Type**: Перечисление, представляющее уровень заряда батареи (например, ОК, предупреждение, критический).
108. **BatReplaceabilityEnum Type**: Перечисление, представляющее возможность замены батареи (например, не подлежит замене, заменяется пользователем, заменяется на заводе).
109. **BatCommonDesignationEnum Type**: Перечисление, представляющее общее обозначение батареи (например, AAA, AA, C).
110. **BatApprovedChemistryEnum Type**: Перечисление, представляющее утвержденный химический состав батареи (например, щелочной, литий-ионный).
111. **BatChargeStateEnum Type**: Перечисление, представляющее состояние заряда батареи (например, неизвестно, заряжается, полностью заряжен).
112. **PowerSourceStatusEnum Type**: Перечисление, представляющее статус источника питания (например, активный, в режиме ожидания, недоступный).
113. **WiredCurrentTypeEnum Type**: Перечисление, представляющее тип проводного тока (например, переменный ток, постоянный ток).

## Structs (C/C++)

* **ProductAppearanceStruct Type:**
  * `Finish`: `ProductFinishEnum`
  * `PrimaryColor`: `ColorEnum`
* **CapabilityMinimaStruct Type:**
  * `CaseSessionsPerFabric`: `uint16`
  * `SubscriptionsPerFabric`: `uint16`
* **GroupKeyMapStruct Type:**
  * `GroupId`: `group-id`
  * `GroupKeySetID`: `uint16`
* **GroupKeySetStruct Type:**
  * `GroupKeySetID`: `uint16`
  * `GroupKeySecurityPolicy`: `GroupKeySecurityPolicyEnum`
  * `EpochKey0`: `octstr16`
  * `EpochStartTime0`: `epoch-us`
  * `EpochKey1`: `octstr16`
  * `EpochStartTime1`: `epoch-us`
  * `EpochKey2`: `octstr16`
  * `EpochStartTime2`: `epoch-us`
  * `GroupKeyMulticastPolicy`: `GroupKeyMulticastPolicyEnum`
* **GroupInfoMapStruct Type:**
  * `GroupId`: `group-id`
  * `Endpoints`: `list[endpoint-no]`
  * `GroupName`: `string`
* **NetworkInfoStruct Type:**
  * `NetworkID`: `octstr`
  * `Connected`: `bool`
* **WiFiInterfaceScanResultStruct Type:**
  * `Security`: `WiFiSecurityBitmap`
  * `SSID`: `octstr`
  * `BSSID`: `octstr`
  * `Channel`: `uint16`
  * `WiFiBand`: `WiFiBandEnum`
  * `RSSI`: `int8`
* **ThreadInterfaceScanResultStruct Type:**
  * `PanId`: `uint16`
  * `ExtendedPanId`: `uint64`
  * `NetworkName`: `string`
  * `Channel`: `uint16`
  * `Version`: `uint8`
  * `ExtendedAddress`: `hwadr`
  * `RSSI`: `int8`
  * `LQI`: `uint8`
* **BasicCommissioningInfo Type:**
  * `FailSafeExpiryLengthSeconds`: `uint16`
  * `MaxCumulativeFailsafeSeconds`: `uint16`
* **NetworkInterface Type:**
  * `Name`: `string`
  * `IsOperational`: `bool`
  * `OffPremiseServicesReachableIPv4`: `bool`
  * `OffPremiseServicesReachableIPv6`: `bool`
  * `HardwareAddress`: `HardwareAddress`
  * `IPv4Addresses`: `list[ipv4addr]`
  * `IPv6Addresses`: `list[ipv6addr]`
  * `Type`: `InterfaceTypeEnum`
* **ThreadMetricsStruct Type:**
  * `ID`: `uint64`
  * `Name`: `string`
  * `StackFreeCurrent`: `uint32`
  * `StackFreeMinimum`: `uint32`
  * `StackSize`: `uint32`
* **NeighborTableStruct Type:**
  * `ExtAddress`: `uint64`
  * `Age`: `uint32`
  * `Rloc16`: `uint16`
  * `LinkFrameCounter`: `uint32`
  * `MleFrameCounter`: `uint32`
  * `LQI`: `uint8`
  * `AverageRssi`: `int8`
  * `LastRssi`: `int8`
  * `FrameErrorRate`: `uint8`
  * `MessageErrorRate`: `uint8`
  * `RxOnWhenIdle`: `bool`
  * `FullThreadDevice`: `bool`
  * `FullNetworkData`: `bool`
  * `IsChild`: `bool`
* **RouteTableStruct Type:**
  * `ExtAddress`: `uint64`
  * `Rloc16`: `uint16`
  * `RouterId`: `uint8`
  * `NextHop`: `uint8`
  * `PathCost`: `uint8`
  * `LQIIn`: `uint8`
  * `LQIOut`: `uint8`
  * `Age`: `uint8`
  * `Allocated`: `bool`
  * `LinkEstablished`: `bool`
* **SecurityPolicy Type:**
  * `RotationTime`: `uint16`
  * `Flags`: `uint16`
* **OperationalDatasetComponents Type:**
  * `ActiveTimestampPresent`: `bool`
  * `PendingTimestampPresent`: `bool`
  * `MasterKeyPresent`: `bool`
  * `NetworkNamePresent`: `bool`
  * `ExtendedPanIdPresent`: `bool`
  * `MeshLocalPrefixPresent`: `bool`
  * `DelayPresent`: `bool`
  * `PanIdPresent`: `bool`
  * `ChannelPresent`: `bool`
  * `PskcPresent`: `bool`
  * `SecurityPolicyPresent`: `bool`
  * `ChannelMaskPresent`: `bool`
* **BasicCommissioningInfo Type:**
  * `FailSafeExpiryLengthSeconds`: `uint16`
  * `MaxCumulativeFailsafeSeconds`: `uint16`
* **TrustedTimeSourceStruct Type:**
  * `FabricIndex`: `fabric-idx`
  * `NodeID`: `node-id`
  * `Endpoint`: `endpoint-no`
* **FabricScopedTrustedTimeSourceStruct Type:**
  * `NodeID`: `node-id`
  * `Endpoint`: `endpoint-no`
* **TimeZoneStruct Type:**
  * `Offset`: `int32`
  * `ValidAt`: `epoch-us`
  * `Name`: `string`
* **DSTOffsetStruct Type:**
  * `Offset`: `int32`
  * `ValidStarting`: `epoch-us`
  * `ValidUntil`: `epoch-us`
* **ProviderLocation Type:**
  * `ProviderNodeID`: `node-id`
  * `Endpoint`: `endpoint-no`
* **FabricDescriptorStruct Type:**
  * `RootPublicKey`: `octstr`
  * `VendorID`: `vendor-id`
  * `FabricID`: `fabric-id`
  * `NodeID`: `node-id`
  * `Label`: `string`
* **DatastoreStatusEntry Type:**
  * `State`: `enum`
  * `UpdateTimeStamp`: `utcall`
* **Datastore Node Key Entry Type:**
  * `GroupKeySetID`: `uint16`
  * `StatusEntry`: `struct`
* **Datastore Group Information Entry Type:**
  * `GroupID`: `uint64`
  * `FriendlyName`: `string`
  * `GroupKeySetID`: `uint16`
  * `GroupCAT`: `uint16`
  * `GroupCATVersion`: `uint16`
  * `GroupPermission`: `enum`
* **Datastore Binding Entry Type:**
  * `ListID`: `uint16`
  * `BindingTargetStruct`: `struct`
  * `StatusEntry`: `struct`
* **Datastore Group ID Entry Type:**
  * `GroupID`: `uint64`
  * `StatusEntry`: `struct`
* **Datastore Endpoint Entry Type:**
  * `EndpointID`: `endpoint-id`
  * `NodeID`: `node-id`
  * `FriendlyName`: `string`
  * `StatusEntry`: `struct`
  * `GroupIDList`: `list`
  * `BindingList`: `list`
* **Datastore ACL Entry Type:**
  * `ListID`: `uint16`
  * `ACLEntry`: `struct`
  * `StatusEntry`: `struct`
* **Datastore Node Information Entry Type:**
  * `NodeID`: `node-id`
  * `FriendlyName`: `string`
  * `CommissioningStatusEntry`: `struct`
  * `NodeKeySetList`: `list`
  * `ACLList`: `list`
  * `EndpointList`: `list`
* **Datastore Administrator Information Entry Type:**
  * `NodeID`: `node-id`
  * `FriendlyName`: `string`
  * `VendorID`: `vendor-id`
  * `ICAC`: `octstr`

## Enums (если имеются)

* **ProductFinishEnum Type:**
  * `Other`
  * `Matte`
  * `Satin`
  * `Polished`
  * `Rugged`
  * `Fabric`
* **ColorEnum Type:**
  * `Black`
  * `Navy`
  * `Green`
  * `Teal`
  * `Maroon`
  * `Purple`
  * `Olive`
  * `Gray`
  * `Blue`
  * `Lime`
  * `Aqua`
  * `Red`
  * `Fuchsia`
  * `Yellow`
  * `White`
  * `Nickel`
  * `Chrome`
  * `Brass`
  * `Copper`
  * `Silver`
  * `Gold`
* **GroupKeySecurityPolicyEnum Type:**
  * `TrustFirst`
  * `CacheAndSync`
* **GroupKeyMulticastPolicyEnum Type:**
  * `PerGroupID`
  * `AllNodes`
* **HourFormatEnum Type:**
  * `12hr`
  * `24hr`
  * `UseActiveLocale`
* **CalendarTypeEnum Type:**
  * `Buddhist`
  * `Chinese`
  * `Coptic`
  * `Ethiopian`
  * `Gregorian`
  * `Hebrew`
  * `Indian`
  * `Islamic`
  * `Japanese`
  * `Korean`
  * `Persian`
  * `Taiwanese`
  * `UseActiveLocale`
* **TempUnitEnum Type:**
  * `Fahrenheit`
  * `Celsius`
  * `Kelvin`
* **WiredFaultEnum Type:**
  * `Unspecified`
  * `OverVoltage`
  * `UnderVoltage`
* **BatFaultEnum Type:**
  * `Unspecified`
  * `OverTemp`
  * `UnderTemp`
* **BatChargeFaultEnum Type:**
  * `Unspecified`
  * `AmbientTooHot`
  * `AmbientTooCold`
  * `BatteryTooHot`
  * `BatteryTooCold`
  * `BatteryAbsent`
  * `BatteryOverVoltage`
  * `BatteryUnderVoltage`
  * `ChargerOverVoltage`
  * `ChargerUnderVoltage`
  * `SafetyTimeout`
* **PowerSourceStatusEnum Type:**
  * `Unspecified`
  * `Active`
  * `Standby`
  * `Unavailable`
* **WiredCurrentTypeEnum Type:**
  * `AC`
  * `DC`
* **BatChargeLevelEnum Type:**
  * `OK`
  * `Warning`
  * `Critical`
* **BatReplaceabilityEnum Type:**
  * `Unspecified`
  * `NotReplaceable`
  * `UserReplaceable`
  * `FactoryReplaceable`
* **BatCommonDesignationEnum Type:**
  * `Unspecified`
  * `AAA`
  * `AA`
  * `C`
  * `D`
  * `4v5`
  * `6v0`
  * `9v0`
  * `1_2AA`
  * `AAAA`
  * `A`
  * `B`
  * `F`
  * `N`
  * `No6`
  * `SubC`
  * `A23`
  * `A27`
  * `BA5800`
  * `Duplex`
  * `4SR44`
  * `523`
  * `531`
  * `15v0`
  * `22v5`
  * `30v0`
  * `45v0`
  * `67v5`
  * `J`
  * `CR123A`
  * `CR2`
  * `2CR5`
  * `CR_P2`
  * `CR_V3`
  * `SR41`
  * `SR43`
  * `SR44`
  * `SR45`
  * `SR48`
  * `SR54`
  * `SR55`
  * `SR57`
  * `SR58`
  * `SR59`
  * `SR60`
  * `SR63`
  * `SR64`
  * `SR65`
  * `SR66`
  * `SR67`
  * `SR68`
  * `SR69`
  * `SR516`
  * `SR731`
  * `SR712`
  * `LR932`
 	* `A5`
  * `A10`
  * `A13`
  * `A312`
  * `A675`
  * `AC41E`
  * `10180`
  * `10280`
  * `10440`
  * `14250`
  * `14430`
  * `14500`
  * `14650`
  * `15270`
  * `16340`
  * `RCR123A`
  * `17500`
  * `17670`
  * `18350`
  * `18500`
  * `18650`
  * `19670`
  * `25500`
  * `26650`
  * `32600`
* **BatApprovedChemistryEnum Type:**
  * `Unspecified`
  * `Alkaline`
  * `LithiumCarbonFluoride`
  * `LithiumChromiumOxide`
  * `LithiumCopperOxide`
  * `LithiumIronDisulfide`
  * `LithiumManganeseDioxide`
  * `LithiumThionylChloride`
  * `Magnesium`
  * `MercuryOxide`
  * `NickelOxyhydride`
  * `SilverOxide`
  * `ZincAir`
  * `ZincCarbon`
  * `ZincChloride`
  * `ZincManganeseDioxide`
  * `LeadAcid`
  * `LithiumCobaltOxide`
  * `LithiumIon`
  * `LithiumIonPolymer`
  * `LithiumIronPhosphate`
  * `LithiumSulfur`
  * `LithiumTitanate`
  * `NickelCadmium`
  * `NickelHydrogen`
  * `NickelIron`
  * `NickelMetalHydride`
  * `NickelZinc`
  * `SilverZinc`
  * `SodiumIon`
  * `SodiumSulfur`
  * `ZincBromide`
  * `ZincCerium`
* **BatChargeStateEnum Type:**
  * `Unknown`
  * `IsCharging`
  * `IsAtFullCharge`
  * `IsNotCharging`
* **WiFiSecurityBitmap Type:**
  * `Unencrypted`
  * `WEP`
  * `WPA-PERSONAL`
  * `WPA2-PERSONAL`
  * `WPA3-PERSONAL`
* **ThreadCapabilitiesBitmap Type:**
  * `IsBorderRouterCapable`
  * `IsRouterCapable`
  * `IsSleepyEndDeviceCapable`
  * `IsFullThreadDevice`
  * `IsSynchronizedSleepyEndDeviceCapable`
* **WiFiBandEnum Type:**
  * `2G4`
  * `3G65`
  * `5G`
  * `6G`
  * `60G`
  * `1G`
* **NetworkCommissioningStatusEnum Type:**
  * `Success`
  * `OutOfRangeValue`
  * `BoundsExceeded`
  * `NetworkIDNotFound`
  * `DuplicateNetworkID`
  * `NetworkNotFound`
  * `RegulatoryError`
  * `AuthFailure`
  * `UnsupportedSecurity`
  * `OtherConnectionFailure`
  * `IPV6Failed`
  * `IPBindFailed`
  * `UnknownError`
* **CommissioningErrorEnum Type:**
  * `OK`
  * `ValueOutsideRange`
  * `InvalidAuthentication`
  * `NoFailSafe`
  * `BusyWithOtherAdmin`
  * `RequiredTCNotAccepted`
  * `TCAcknowledgementsNotReceived`
  * `TCMinVersionNotMet`
* **RegulatoryLocationTypeEnum Type:**
  * `Indoor`
  * `Outdoor`
  * `IndoorOutdoor`
* **HourFormatEnum Type**
  * `12hr`
  * `24hr`
  * `UseActiveLocale`
* **CommissioningErrorEnum Type**
  * `OK`
  * `ValueOutsideRange`
  * `InvalidAuthentication`
  * `NoFailSafe`
  * `BusyWithOtherAdmin`
  * `RequiredTCNotAccepted`
  * `TCAcknowledgementsNotReceived`
  * `TCMinVersionNotMet`
* **RegulatoryLocationTypeEnum Type**
  * `Indoor`
  * `Outdoor`
  * `IndoorOutdoor`
* **TempUnitEnum Type**
  * `Fahrenheit`
  * `Celsius`
  * `Kelvin`
* **PowerSourceStatusEnum Type**
  * `Unspecified`
  * `Active`
  * `Standby`
  * `Unavailable`
* **WiredCurrentTypeEnum Type**
  * `AC`
  * `DC`
* **BatChargeLevelEnum Type**
  * `OK`
  * `Warning`
  * `Critical`
* **BatReplaceabilityEnum Type**
  * `Unspecified`
  * `NotReplaceable`
  * `UserReplaceable`
  * `FactoryReplaceable`
* **BatCommonDesignationEnum Type**
  * `Unspecified`
  * `AAA`
  * `AA`
  * `C`
  * `D`
  * `4v5`
  * `6v0`
  * `9v0`
  * `1_2AA`
  * `AAAA`
  * `A`
  * `B`
  * `F`
  * `N`
  * `No6`
  * `SubC`
  * `A23`
  * `A27`
  * `BA5800`
  * `Duplex`
  * `4SR44`
  * `523`
  * `531`
  * `15v0`
  * `22v5`
  * `30v0`
  * `45v0`
  * `67v5`
  * `J`
  * `CR123A`
  * `CR2`
  * `2CR5`
  * `CR_P2`
  * `CR_V3`
  * `SR41`
  * `SR43`
  * `SR44`
  * `SR45`
  * `SR48`
  * `SR54`
  * `SR55`
  * `SR57`
  * `SR58`
  * `SR59`
  * `SR60`
  * `SR63`
  * `SR64`
  * `SR65`
  * `SR66`
  * `SR67`
  * `SR68`
  * `SR69`
  * `SR516`
  * `SR731`
  * `SR712`
  * `LR932`
  * `A5`
  * `A10`
  * `A13`
  * `A312`
  * `A675`
  * `AC41E`
  * `10180`
  * `10280`
  * `10440`
  * `14250`
  * `14430`
  * `14500`
  * `14650`
  * `15270`
  * `16340`
  * `RCR123A`
  * `17500`
  * `17670`
  * `18350`
  * `18500`
  * `18650`
  * `19670`
  * `25500`
  * `26650`
  * `32600`
* **BatApprovedChemistryEnum Type**
  * `Unspecified`
  * `Alkaline`
  * `LithiumCarbonFluoride`
  * `LithiumChromiumOxide`
  * `LithiumCopperOxide`
  * `LithiumIronDisulfide`
  * `LithiumManganeseDioxide`
  * `LithiumThionylChloride`
  * `Magnesium`
  * `MercuryOxide`
  * `NickelOxyhydride`
  * `SilverOxide`
  * `ZincAir`
  * `ZincCarbon`
  * `ZincChloride`
  * `ZincManganeseDioxide`
  * `LeadAcid`
  * `LithiumCobaltOxide`
  * `LithiumIon`
  * `LithiumIonPolymer`
  * `LithiumIronPhosphate`
  * `LithiumSulfur`
  * `LithiumTitanate`
  * `NickelCadmium`
  * `NickelHydrogen`
  * `NickelIron`
  * `NickelMetalHydride`
  * `NickelZinc`
  * `SilverZinc`
  * `SodiumIon`
  * `SodiumSulfur`
  * `ZincBromide`
  * `ZincCerium`
* **BatChargeStateEnum Type**
  * `Unknown`
  * `IsCharging`
  * `IsAtFullCharge`
  * `IsNotCharging`
* **WiFiSecurityBitmap Type**
  * `Unencrypted`
  * `WEP`
  * `WPA-PERSONAL`
  * `WPA2-PERSONAL`
  * `WPA3-PERSONAL`
* **ThreadCapabilitiesBitmap Type**
  * `IsBorderRouterCapable`
  * `IsRouterCapable`
  * `IsSleepyEndDeviceCapable`
  * `IsFullThreadDevice`
  * `IsSynchronizedSleepyEndDeviceCapable`
* **WiFiBandEnum Type**
  * `2G4`
  * `3G65`
  * `5G`
  * `6G`
  * `60G`
  * `1G`
* **NetworkCommissioningStatusEnum Type**
  * `Success`
  * `OutOfRangeValue`
  * `BoundsExceeded`
  * `NetworkIDNotFound`
  * `DuplicateNetworkID`
  * `NetworkNotFound`
  * `RegulatoryError`
  * `AuthFailure`
  * `UnsupportedSecurity`
  * `OtherConnectionFailure`
  * `IPV6Failed`
  * `IPBindFailed`
  * `UnknownError`
* **CommissioningErrorEnum Type**
  * `OK`
  * `ValueOutsideRange`
  * `InvalidAuthentication`
  * `NoFailSafe`
  * `BusyWithOtherAdmin`
  * `RequiredTCNotAccepted`
  * `TCAcknowledgementsNotReceived`
  * `TCMinVersionNotMet`
* **RegulatoryLocationTypeEnum Type**
  * `Indoor`
  * `Outdoor`
  * `IndoorOutdoor`
* **HardwareFaultEnum Type:**
  * `Unspecified`
  * `Radio`
  * `Sensor`
  * `ResettableOverTemp`
  * `NonResettableOverTemp`
  * `PowerSource`
  * `VisualDisplayFault`
  * `AudioOutputFault`
  * `UserInterfaceFault`
  * `NonVolatileMemoryError`
  * `TamperDetected`
* **RadioFaultEnum Type:**
  * `Unspecified`
  * `WiFiFault`
  * `CellularFault`
  * `ThreadFault`
  * `NFCFault`
  * `BLEFault`
  * `EthernetFault`
* **NetworkFaultEnum Type:**
  * `Unspecified`
  * `HardwareFailure`
  * `NetworkJammed`
  * `ConnectionFailed`
* **InterfaceTypeEnum Type:**
  * `Unspecified`
  * `WiFi`
  * `Ethernet`
  * `Cellular`
  * `Thread`
* **BootReasonEnum Type:**
  * `Unspecified`
  * `PowerOnReboot`
  * `BrownOutReset`
  * `SoftwareWatchdogReset`
  * `HardwareWatchdogReset`
  * `SoftwareUpdateCompleted`
  * `SoftwareReset`
* **TempUnitEnum Type:**
  * `Fahrenheit`
  * `Celsius`
  * `Kelvin`
* **PowerSourceStatusEnum Type:**
  * `Unspecified`
  * `Active`
  * `Standby`
  * `Unavailable`
* **WiredCurrentTypeEnum Type:**
  * `AC`
  * `DC`
* **BatChargeLevelEnum Type:**
  * `OK`
  * `Warning`
  * `Critical`
* **BatReplaceabilityEnum Type:**
  * `Unspecified`
  * `NotReplaceable`
  * `UserReplaceable`
  * `FactoryReplaceable`
* **BatCommonDesignationEnum Type:** (много значений, смотрите в оригинальном тексте)
* **BatApprovedChemistryEnum Type:** (много значений, смотрите в оригинальном тексте)
* **BatChargeStateEnum Type:**
  * `Unknown`
  * `IsCharging`
  * `IsAtFullCharge`
  * `IsNotCharging`
* **WiFiSecurityBitmap Type:**
  * `Unencrypted`
  * `WEP`
  * `WPA-PERSONAL`
  * `WPA2-PERSONAL`
  * `WPA3-PERSONAL`
* **WiFiBandEnum Type:**
  * `2G4`
  * `3G65`
  * `5G`
  * `6G`
  * `60G`
  * `1G`
* **NetworkCommissioningStatusEnum Type:**
  * `Success`
  * `OutOfRange`
  * `BoundsExceeded`
  * `NetworkIDNotFound`
  * `DuplicateNetworkID`
  * `NetworkNotFound`
  * `RegulatoryError`
  * `AuthFailure`
  * `UnsupportedSecurity`
  * `OtherConnectionFailure`
  * `IPV6Failed`
  * `IPBindFailed`
  * `UnknownError`
* **CommissioningErrorEnum Type:**
  * `OK`
  * `ValueOutsideRange`
  * `InvalidAuthentication`
  * `NoFailSafe`
  * `BusyWithOtherAdmin`
  * `RequiredTCNotAccepted`
  * `TCAcknowledgementsNotReceived`
  * `TCMinVersionNotMet`
* **RegulatoryLocationTypeEnum Type:**
  * `Indoor`
  * `Outdoor`
  * `IndoorOutdoor`
* **HourFormatEnum Type:**
  * `12hr`
  * `24hr`
  * `UseActiveLocale`
* **TempUnitEnum Type:**
  * `Fahrenheit`
  * `Celsius`
  * `Kelvin`
* **WiredFaultEnum Type:**
  * `Unspecified`
  * `OverVoltage`
  * `UnderVoltage`
* **BatFaultEnum Type:**
  * `Unspecified`
  * `OverTemp`
  * `UnderTemp`
* **BatChargeFaultEnum Type:**
  * `Unspecified`
  * `AmbientTooHot`
  * `AmbientTooCold`
  * `BatteryTooHot`
  * `BatteryTooCold`
  *
