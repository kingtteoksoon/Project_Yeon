# 프로젝트 연(緣) — Yeon

Reverie Studio 1인 개발. Unreal Engine 5.7. 한국 신화 3D 액션.

## 두 트랙

| | 졸업작 | 완성작 (본게임) |
|---|---|---|
| 시점 | 946 CE | 1401 CE |
| 산출물 | 보스전 버티컬 슬라이스 + 트레일러 | 본게임 전체 |
| 마감 | 2027 Q4 (1차) / 2028 (최종) | 2029~ |
| Steam 출시 | 없음 | 있음 |

현재 작업 단계: **졸업작 기획 완성** (개발 착수는 2027 Q2, 전역 후)

## 빌드 타겟

- Ryzen 5600X / 16GB / RTX 3060 Ti, FHD 60fps
- 개발: Mac mini (기획) + Windows PC RTX 5070 Ti (UE5)

## Content 폴더 구조

GDD 15.3 기준.

```
Content/
├── _Core/               # GameMode, GameInstance 등 핵심 로직 애셋
├── Characters/
│   ├── Baekyon/         # 백연 — Meshes, Animations, Materials, Blueprints
│   └── Imugi/           # 이무기 — Meshes, Animations, Materials, Blueprints
├── Levels/
│   ├── ZoneC/           # Zone C 맵
│   └── Boss/            # 보스 아레나
├── UI/
│   ├── HUD/             # HP 바, 강공 차징 게이지, 보스 HP
│   └── Widgets/
├── VFX/
│   ├── Baekyon/         # 검기, Dash 잔상
│   ├── Imugi/           # 청록/적자색 발광
│   └── Finisher/        # 피니시 발광
├── Audio/
│   ├── BGM/             # 1페이즈, 2페이즈, 피니시 트랙
│   └── SFX/             # 타격음, 예고 신호 등
└── Env/
    ├── ZoneC/           # 환경 애셋
    ├── Materials/
    └── Textures/
```

## Source 폴더 구조

Blueprint + C++ 병행. 초기 프로토타입은 Blueprint 우선, 이후 성능/구조 필요 모듈을 C++ 전환.

```
Source/Project_Yeon/
├── Core/            # GameMode, GameInstance
├── Characters/
│   ├── Baekyon/     # 백연 캐릭터 클래스
│   └── Imugi/       # 이무기 보스 클래스
├── Combat/          # 전투 시스템, 히트박스, 히트스톱
├── Camera/          # SpringArm 기반 + 락온 + 피니시 절차적 카메라
└── UI/              # HUD, 위젯
```

## 전투 시스템 핵심

- **루프:** 읽기 → Dash → 콤보 → 피니시
- **입력:** 좌클릭(약공격 1-2-3타), 우클릭 홀드(강공격 차징), Shift/Space(Dash)
- **콤보 분기 트리 없음** — 약·강 자유 혼용만 (D-11 폐기)
- **Dash:** 무적 0.18초, 400cm, 쿨다운 0.4초. 약공격 3타 완주 시 즉시 쿨다운 초기화
- **보스 AI:** State Machine (Blueprint Enum + Switch). Behavior Tree 불사용

## 확정 결정 요약 (D-코드)

- **D-01:** 졸업작 Steam 유료 출시 없음
- **D-02:** 여우구슬 — 졸업작=Niagara 연출 전용 / 완성작=게임플레이 메커닉
- **D-05:** 완성작 맵 곳곳 체크포인트 / 졸업작=보스 아레나 입구 단일
- **D-11:** 콤보 분기 트리 폐기
- **G-01:** 완성작에 "연" 고유 메커닉 1개 이상 추가
- **G-03:** 완성작에서 산군 별도 연출 추가

## 기본 원칙

1. 모든 보스 공격은 명시적 예고 모션을 가진다
2. 강한 연출(카메라 흔들림, 슬로우모션, Niagara 폭발)은 페이즈 전환과 피니시에만 한정
3. 여우구슬은 졸업작 범위에서 게임플레이 로직으로 확장하지 않는다
4. 기능 추가 전 "플레이어가 실제로 느낄 수 있는가" 확인 — 느낄 수 없으면 Cut 후보

## GDD 위치

Notion: https://app.notion.com/p/36b0aab3887781478c29e120e6d6c07f

확정/미확정 페이지가 항상 최신 권위 소스. 충돌 시 Notion 페이지 우선.
