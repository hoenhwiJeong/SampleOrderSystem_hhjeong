\# Clean Code Guide



프로젝트: Semiconductor MES Console System



목표:



\* 읽기 쉬운 코드

\* 유지보수 가능한 구조

\* 테스트 가능한 설계

\* 확장 가능한 아키텍처

\* Claude AI 협업 최적화



\---



\# 1. 기본 원칙



\## 1.1 코드는 사람이 읽기 위해 작성한다



컴파일러보다 개발자를 우선한다.



나쁜 예



```cpp

int a;

```



좋은 예



```cpp

int totalOrderCount;

```



\---



\## 1.2 의미 있는 이름 사용



이름만 보고 역할을 알 수 있어야 한다.



나쁜 예



```cpp

int x;

int data;

```



좋은 예



```cpp

int inventoryCount;

int pendingOrderCount;

```



\---



\## 1.3 축약어 사용 금지



허용



```cpp

MES

DTO

API

UI

ID

```



금지



```cpp

InvMgr

ProdSvc

OrdProc

```



좋은 예



```cpp

InventoryManager

ProductionService

OrderProcessor

```



\---



\# 2. 함수 작성 원칙



\## 2.1 함수는 한 가지 일만 수행한다



나쁜 예



```cpp

void ProcessOrder()

{

&#x20;   ValidateOrder();

&#x20;   UpdateInventory();

&#x20;   SaveDatabase();

&#x20;   SendNotification();

}

```



좋은 예



```cpp

void ProcessOrder();



bool ValidateOrder();

void ReserveInventory();

void SaveOrder();

void NotifyCustomer();

```



\---



\## 2.2 함수 길이 제한



권장



```text

10\~30 라인

```



최대



```text

50 라인

```



초과 시 분리 고려



\---



\## 2.3 함수 이름은 동사로 시작



예시



```cpp

CreateOrder()

ApproveOrder()

RejectOrder()

LoadInventory()

RenderDashboard()

```



\---



\# 3. 클래스 작성 원칙



\## 3.1 클래스는 하나의 책임만 가진다



SRP (Single Responsibility Principle)



좋은 예



```cpp

InventoryManager

OrderManager

ProductionManager

```



나쁜 예



```cpp

MESSystemManager

```



모든 기능이 몰려 있는 클래스



\---



\## 3.2 클래스 크기 제한



권장



```text

300줄 이하

```



최대



```text

500줄 이하

```



초과 시 분리



\---



\## 3.3 God Object 금지



나쁜 예



```cpp

SystemManager

```



담당



\* 주문

\* 재고

\* 생산

\* 출고

\* UI



전부 처리



\---



\# 4. SOLID 원칙



\## S - Single Responsibility



하나의 클래스는 하나의 책임만 가진다.



\---



\## O - Open Closed



확장에는 열려있고 수정에는 닫혀있어야 한다.



\---



\## L - Liskov Substitution



부모 타입을 자식 타입으로 안전하게 교체 가능해야 한다.



\---



\## I - Interface Segregation



사용하지 않는 인터페이스를 강요하지 않는다.



\---



\## D - Dependency Inversion



구체 클래스가 아닌 추상화에 의존한다.



좋은 예



```cpp

IInventoryRepository

```



\---



\# 5. UI 코드 규칙



\## View 와 Business Logic 분리



금지



```cpp

DashboardView

{

&#x20;   SQL 실행

&#x20;   재고 계산

&#x20;   주문 처리

}

```



허용



```cpp

DashboardView

{

&#x20;   Render();

}

```



비즈니스 로직은 Service 계층에서 수행



\---



\## Console 출력 직접 사용 최소화



금지



```cpp

std::cout << "Order";

```



권장



```cpp

renderer.DrawText();

renderer.DrawPanel();

renderer.DrawTable();

```



\---



\# 6. 예외 처리



\## 예외 무시 금지



나쁜 예



```cpp

catch (...)

{

}

```



\---



\## 로그 기록



좋은 예



```cpp

catch (const std::exception\& ex)

{

&#x20;   logger.Error(ex.what());

}

```



\---



\# 7. 주석 규칙



\## 코드 설명 주석 금지



나쁜 예



```cpp

// i를 1 증가

i++;

```



\---



\## 의도 설명 주석 허용



좋은 예



```cpp

// 생산 예약 수량은 실제 재고에서 제외하지 않는다.

```



\---



\# 8. 하드코딩 금지



나쁜 예



```cpp

if (stock < 100)

```



좋은 예



```cpp

constexpr int LOW\_STOCK\_THRESHOLD = 100;

```



\---



\# 9. 매직 넘버 금지



나쁜 예



```cpp

if (status == 3)

```



좋은 예



```cpp

if (status == OrderStatus::Producing)

```



\---



\# 10. 프로젝트 구조



권장 구조



```text

src/

├─ domain/

│

├─ application/

│

├─ infrastructure/

│

├─ ui/

│

├─ tests/

│

└─ main.cpp

```



\---



\# 11. 네이밍 규칙



\## 클래스



PascalCase



```cpp

InventoryManager

OrderService

DashboardView

```



\---



\## 함수



PascalCase



```cpp

CreateOrder()

ApproveOrder()

LoadInventory()

```



\---



\## 변수



camelCase



```cpp

orderCount

inventoryQuantity

pendingOrders

```



\---



\## 상수



UPPER\_CASE



```cpp

MAX\_ORDER\_COUNT

LOW\_STOCK\_THRESHOLD

```



\---



\# 12. 테스트 규칙



모든 핵심 로직은 테스트 가능해야 한다.



예시



```cpp

InventoryManagerTests

OrderManagerTests

ProductionManagerTests

```



\---



\# 13. Claude 협업 규칙



Claude가 생성하는 모든 코드는 아래를 준수한다.



\* SOLID 원칙 준수

\* SRP 준수

\* 하드코딩 금지

\* 매직 넘버 금지

\* View와 Logic 분리

\* 함수 50라인 이하

\* 클래스 500라인 이하

\* 의미 있는 네이밍 사용

\* 테스트 가능한 구조 유지



코드 생성 시 항상:



1\. 설계 설명

2\. 클래스 다이어그램

3\. 구현 코드



순서로 작성한다.



\---



\# 최종 목표



읽기 쉬운 코드



변경하기 쉬운 코드



테스트하기 쉬운 코드



확장하기 쉬운 코드



Claude와 사람이 함께 유지보수 가능한 코드



