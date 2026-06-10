# 프로젝트 연(緣) — Yeon

Reverie Studio 1인 개발. **Unreal Engine 5.7** 기반 한국 신화 3D 액션 게임.

## 개요

| | 졸업작 | 완성작 (본게임) |
|---|---|---|
| 시점 | 946 CE | 1401 CE |
| 산출물 | 보스전 버티컬 슬라이스 + 트레일러 | 본게임 전체 |
| 마감 | 2027 Q4 (1차) / 2028 (최종) | 2029~ |
| Steam 출시 | 없음 | 있음 |

현재 단계: **졸업작 기획 완성** (개발 착수 2027 Q2)

## 빌드 타겟

- Ryzen 5600X / 16GB / RTX 3060 Ti — FHD 60fps
- 개발 환경: Mac mini (기획) + Windows PC RTX 5070 Ti (UE5)

## 전투 시스템

- **루프:** 읽기 → Dash → 콤보 → 피니시
- **입력:** 좌클릭(약공격 1-2-3타), 우클릭 홀드(강공격 차징), Shift/Space(Dash)
- **Dash:** 무적 0.18초, 400cm, 쿨다운 0.4초. 약공격 3타 완주 시 즉시 쿨다운 초기화
- **보스 AI:** State Machine (Blueprint Enum + Switch), Behavior Tree 불사용

## 구현 현황

### 캐릭터 — 백연 (`Source/Project_Yeon/Characters/Baekyon/`)

- **`BaekYonCharacter`** — 플레이어 캐릭터
  - SpringArm + FollowCamera (카메라 랙 적용, 이동 방향 회전)
  - Enhanced Input: 이동 / 시점 / 점프 / Dash / 뛰기(홀드)
  - 이동 속도: 기본 걷기 200, 뛰기 홀드 시 600
  - Dash: `LaunchCharacter` 순간 추진 + 무적/쿨다운 타이머 (GDD 7.3)
- **`BaekYonAnimInstance`** — 로코모션 애님 인스턴스
  - `GroundSpeed`, `bShouldMove`, `bIsFalling` 를 C++에서 계산해 AnimGraph로 전달

### Enhanced Input 바인딩

| C++ 프로퍼티 | 역할 | 에셋 |
|---|---|---|
| `MoveAction` | WASD 이동 | `IA_Move` |
| `LookAction` | 마우스 시점 | `IA_Look` |
| `RunAction` | 홀드 시 뛰기 | `IA_Walk` |
| `JumpAction` | 점프 | `IA_Jump` |
| `DashAction` | Dash | `IA_Dash` |

> IMC의 WASD 키는 모디파이어 필요 — W: Swizzle(YXZ) / S: Swizzle(YXZ)+Negate / A: Negate / D: 없음

### 애니메이션 (`Content/Characters/Baekyon/`)

- Blend Space 1D 로코모션 (`BS_BaekYonLocomotion`) — Walking / Fast_Run
- State Machine: Ground ↔ Air (`bIsFalling` 전환), Jump_Anim
- 셋업 다이어그램: `Docs/ABP_BaekYon_Setup.svg`
- ⚠️ Idle 애니메이션 미보유 — 현재 속도 0에서 Walking 재생(임시)

## 폴더 구조

```
Source/Project_Yeon/
├── Core/            # GameMode, GameInstance
├── Characters/      # Baekyon(플레이어), Imugi(보스)
├── Combat/          # 전투, 히트박스, 히트스톱
├── Camera/          # 락온 + 피니시 카메라
└── UI/              # HUD, 위젯
```

자세한 Content/Source 구조는 [`CLAUDE.md`](CLAUDE.md) 참조.

## 개발 방침

- 구현은 **C++ 위주** (GDD 15.2 — 포트폴리오 평가 대상)
- 모든 보스 공격은 명시적 예고 모션을 가진다
- 강한 연출(카메라 흔들림, 슬로우모션, Niagara)은 페이즈 전환·피니시에만 한정
- 기능 추가 전 "플레이어가 실제로 느낄 수 있는가" 확인

## GDD

Notion (확정/미확정 페이지가 최신 권위 소스): https://app.notion.com/p/36b0aab3887781478c29e120e6d6c07f
