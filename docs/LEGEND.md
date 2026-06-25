# Oikumene 图例说明

当前版本使用程序化图标，不引入正式美术素材。窗口内按 `F2` 可以打开图例面板。

## 地形

- Ocean：深海。早期不可通行，会阻断控制力和陆路扩张。
- Coast：海岸。适合水产、未来港口和沿海路线。
- Grassland：草地。适合牧场、早期迁徙和部分农田。
- Forest / Rainforest：森林和雨林。提供木材，但会提高移动和治理成本。
- Wetland：湿地。水源和肥沃度较好，但通行成本高。
- Desert / Tundra / Snow：沙漠、冻原、雪地。农业和人口承载力较低，控制成本较高。
- Hill / Mountain：丘陵和山地。更容易出现矿产和防御优势，但会阻隔国家控制力。

## 水文

- River：河流覆盖线。提高定居点评分，并作为控制力扩散和未来贸易路线的天然走廊。
- Lake：湖泊。提供水源，但类似海洋，会阻断陆地控制力。
- Coastline：海岸线。后续航海、港口和跨海贸易会围绕它扩展。

## 资源

- Wood / Bamboo：建筑、工具和燃料资源。
- Horse：移动力、贸易速度和未来军事机动性的基础。
- Copper / Tin：青铜技术路线的关键矿产。
- Shallow Iron / Shallow Coal：早期可触及的铁和燃料资源。
- Meteoric Iron：极稀有早期高价值金属。
- Gold / Silver / Salt：财富、贸易和政治价值资源。
- Stone / Clay：建筑、陶器、城墙和基础设施资源。

## 改良

- Farm：农田。提高食物产出和本地人口承载力。
- Lumber Camp：伐木场。提高木材产出。
- Pasture：牧场。利用草地、马匹等开阔生态资源。
- Shallow Mine：浅层矿井。利用浅层矿产，支撑后续科技和装备。
- Foraging Ground：采集地。早期过渡性生存改良。

## 路线

- Trail：早期小径。成本低、收益弱，主要用于首都和成员村庄的基本连接。
- Road：道路。需要 Roads 科技，显著降低同 polity 的路径/控制成本，并缓解行政距离负担。
- RiverRoute：河流路线。利用河谷天然廊道连接聚落或矿点。
- CoastalRoute：沿海路线。需要 Sailing 条件，利用海岸连接沿海目标。

路线不是贸易系统本身。当前版本只把路线作为基础设施层：它影响控制力扩散、行政距离、矿点转运和维护成本；贸易系统会读取这套网络来计算首都间贸易成本和效率。需要审计路线效果时，可以用 `--disable-routes` 跑同一批 seed 的对照实验。

## 贸易

- TradeNetwork：活跃贸易协议的首都间路径覆盖层。线条越亮代表当前协议路线效率越高；黄色端点表示贸易双方首都。
- Trade Agreement：两个 polity 之间的贸易协议，记录双方输出货物、互补评分、路径成本、净收益、路径 tile 和连续疲弱刷新次数。
- Weak Refresh：协议低于开约门槛但仍满足续约条件的连续刷新次数；连续多轮疲弱后协议才会关闭。

## 外交

- Friendly：贸易收益、路线效率和长期协议带来的友好关系。
- Competitive：共享边界、争议区、经济结构重叠和军力接近带来的竞争关系。
- Dependent：一方从另一方进口价值明显更高，形成依赖和潜在外交杠杆。
- BlockadeRisk：依赖、竞争、路线脆弱性和弱势贸易叠加后的封锁风险倾向。
- `Dip F/C/D/B`：HUD 中的外交关系摘要，分别表示 Friendly、Competitive、Dependent、BlockadeRisk 数量。
- `G/V/R`：外交详情中的长期记忆摘要，分别表示 Grievance（怨恨）、Vassalage（附庸/依赖记忆）和 Restraint（克制/避免再战倾向）的最大强度。
- TerritoryCeded / OccupationWithdrawn / VassalCreated / OccupationRevolt 会写回外交长期记忆，并随时间衰减。

## 战争压力

当前版本的战争系统执行战略层抽象战争，不做逐格战棋。

- WarPressure：从一个 polity 指向另一个 polity 的战争倾向候选，来源于外交、贸易、边界和军力差异。
- BorderDispute：边界摩擦和竞争主导的冲突倾向。
- TradeCoercion：贸易互补、路线脆弱或经济竞争带来的强制贸易倾向。
- Blockade：有杠杆的一方利用对方依赖和路线脆弱性制造封锁压力。
- DependencyBreakout：依赖方因为受制于对方贸易杠杆而产生摆脱依赖的冲突压力。
- Friendly Penalty：友好关系对宣战倾向的抑制；贸易越稳定，惩罚越明显。
- Trade Conflict Weight：依赖、封锁风险、路线脆弱性和竞争叠加后的贸易冲突权重。
- Grievance Pressure：长期怨恨对宣战倾向和目标价值的推动。
- Restraint Pressure：撤军、代价过高或长期疲惫留下的克制记忆，会压低宣战倾向并提高战役成本。
- Vassalage Pressure：附庸/依赖记忆产生的压力；被支配方更容易出现摆脱依赖的战争倾向，宗主方则更不倾向直接吞并附庸。
- Declaration Pressure：归一化后的宣战压力，仅用于观察和后续战争目标选择。
- WarTargetCandidate：具体战争目标候选，从 WarPressure 推导，包含目标 tile、路径、价值、成本和 ROI。
- Settlement：夺取敌方村庄或首都的目标。
- ResourceRegion：夺取铜、锡、铁、煤、马、盐、金银等资源区域的目标。
- ContestedBorder：争夺双方控制力接近的边境 tile。
- TradeRouteNode：攻击贸易路径节点，用于封锁、强制贸易或摆脱依赖。
- StrategicPass：争夺山口、河谷、沿海路线和其他交通瓶颈。
- Campaign Cost：战役总成本，拆分为动员、补给、装备、地形损耗、防御和占领维护。
- Occupation Cost：占领后的维护压力；距离、目标稳定性和友好惩罚会让占领更不划算。
- WarCampaign：由高分 WarTargetCandidate 转成的抽象战役，记录状态、进度、消耗和结果。
- Active / Occupied / Withdrawn / Peace：战争状态，分别表示进行中、占领目标、撤退失败和议和结束。
- Population Lost / Food Spent / Equipment Spent：战争执行消耗，直接从成员 settlement 的人口和库存中扣除。
- WarDeclared / WarTargetOccupied / WarRetreated / PeaceSigned：战争事件日志，用于复盘每次开战和战后结果。
- OccupationRecord：成功占领后的长期维护记录，记录维护成本、累计短缺、动荡、整合度、叛乱风险和边界稳定压力。
- Active / Ceded / Withdrawn / Vassalized / Revolted：占领状态，分别表示维护中、割让整合、主动撤军、转为附庸缓冲和边疆叛乱。
- TerritoryCeded / OccupationWithdrawn / VassalCreated / OccupationRevolt：战后占领结算事件，用于复盘占领是否真正改变长期边界。

## 实体

- Band：流动狩猎采集群体。
- Camp：早期定居点。
- Village：成熟村庄，可以形成或加入 polity。
- Capital：政治共同体的首都，提供最强控制力源。

## 政治覆盖层

- Controlled Tile：当前由某个 polity 控制力最强的陆地。
- Contested Tile：多个 polity 控制力接近的争议区。
- Capital Ring：首都标记。
- Selection / Hover：当前选中对象或鼠标悬停 tile。

## 科技

科技属于 polity，不属于单个村庄。当前版本先用规则 AI 选择研究方向，后续再交给 Python/LLM 做宏观决策。

- Pottery：提高储粮能力、降低饥荒损失，并小幅提高承载力。
- Irrigation：提高农田产出，河流旁农田额外受益。
- Animal Husbandry：提高牧场产出和马资源价值。
- Mining：让 ShallowMine 正式产生 ore income。
- Roads：允许建设 Road，并降低控制力路径成本和距离行政负担。
- Administration：提高行政容量，并降低 overextension 压力。
- Bronze Working：需要 Mining 和金属条件，提高工具效率和未来军事潜力。
- Fortification：提高防御潜力，并降低争议区对稳定度的损害。
- Sailing：需要海岸接触，解锁未来沿海贸易能力，并小幅加强海岸控制。

## 快捷键

- `F2`：打开或关闭图例。
- `1`-`9`、`0`：切换地图图层，其中 `9` 是 RouteNetwork，`0` 是 TradeNetwork。
- `Tab`：打开详情面板。
- `E`：打开事件面板。
- `C`：居中到当前选中对象。
- `Home` / `F`：适配整张地图。
