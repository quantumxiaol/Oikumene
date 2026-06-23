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

路线不是贸易系统本身。当前版本只把路线作为基础设施层：它影响控制力扩散、行政距离、矿点转运和维护成本；贸易路线会在后续阶段建立在这套网络之上。

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
- `1`-`9`：切换地图图层，其中 `9` 是 RouteNetwork。
- `Tab`：打开详情面板。
- `E`：打开事件面板。
- `C`：居中到当前选中对象。
- `Home` / `F`：适配整张地图。
