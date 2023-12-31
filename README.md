# feverTetris

## File Description
이 레포지토리는 2023-2 SSHS 고급프로그래밍 수행평가 "C언어를 활용한 테트리스의 제작"에서 만들어진 변형 테트리스 "**feverTetris**"의 개발 과정을 포함한 코드입니다.

- 01_fever_tetris : 기본 테트리스 구현 코드
- 02_key_shuffle : key bind shuffle이 포함된 코드
- 03_board_rotation : board rotation이 포함된 코드
- 04_small_block : small block이 포함된 코드
- 05_millisecond : 시간 단위가 1초였던 이슈를 해결한 코드
- 06_fever_time.cpp : 피버 타임을 추가하며 다양한 버그를 해결한 코드

06_fever_time.cpp를 제외한 다른 코드에는 버그가 존재할 수 있음에 유의하기 바랍니다.

## How to Play
올라가는 피버 게이지에 의해 방해 요소가 발동합니다! 최대한 오래 생존하여 높은 점수를 얻으세요!

1. 피버 게이지 : 피버 게이지는 1/30초마다 위로 남은 여유 줄 수에 5를 더한 값만큼 증가합니다. 이 값이 기준치를 넘어가게 되면 피버 타임이 발동됩니다.
2. 피버 타임 : 피버 타임에는 점수가 두 배이지만 세 종류의 방해 요소 중 하나가 발동합니다.
3. 방해 요소 1 : 블록 대신 보드가 회전합니다! 전체 스크린에 대한 블록은 모양이 변하지 않지만 전체 스크린에 대해 보드가 회전함으로써 블록을 회전합니다.
4. 방해 요소 2 : 키 중 두 개라 랜덤하게 바뀌거나 하나의 키의 사용이 제한됩니다.
5. 방해 요소 3 : 고스트가 사라지고, 블록이 사각형 모양에서 점으로 바뀝니다.

키에 따른 블록의 이동은 다음과 같습니다:
| 동작 | 왼쪽 이동  | 오른쪽 이동  | 소프트 드랍 | 하드 드랍 | 시계 방향 회전 | 반시계 방향 회전 | 홀드  |
|----|--------|---------|--------|-------|----------|-----------|-----|
| 키  | 왼쪽 화살표 | 오른쪽 화살표 | 아래 화살표 | 스페이스  | X 키      | Z 키       | C 키 |

## Implementation Details

1. 더블 버퍼링
- Flicking 현상의 발생을 방지하기 위해 더블 버퍼링을 사용했습니다. 더블 버퍼링 설정은 ```namespace doubleConsole```에서 이루어집니다. ```doubleConsole::console```을 통해 콘솔을 불러올 수 있습니다.
2. 랜더링
- 모든 랜더링은 ```printboard()``` 함수에서 이루어집니다. 또한 랜더링 코드의 간결화를 위해 다음 함수들이 구현되었습니다:
  - ```_cout(int, int, char)```: 기본 출력 방식은 하나의 문자를 원하는 위치에 출력하는 것입니다. 그 과정을 함수로 묶어 구현했습니다.
  - ```_wcout(int, int, wchar_t)```: 유니코드 출력을 위해서는 유니코드 문자 전용 타입인 ```wchar_t```를 사용해야 합니다. 유니코드를 출력하는 함수입니다.
  - ```printstr(int, int, char*)```: 시작 위치와 문자열을 입력하면 가로로 문자열을 출력하는 함수입니다.
  - ```printint(int, int, long long)```: 시작 위치와 정수를 입력하면 정수형 변수를 출력하는 함수입니다. 오버플로우 방지를 위해 ```long long``` 타입으로 설정했습니다.
  - ```printline(int, int, int, int, char)```: 줄의 양 끝 위치를 입력하면 가로줄 또는 세로줄일 때 모든 칸에 입력받은 문자를 출력합니다.
  - ```printrect(int, int, int, int)```: 주어진 두 꼭짓점을 양 끝으로 하는 사각형을 출력합니다. ```printline```으로 구현되었습니다.
- 다음 블록과 홀드 블록의 표시를 위해 ```const int ch[][][]```가 만들어졌습니다. 다음 블록과 홀드 블록을 표시할 때 블록이 있는 칸은 1, 없는 칸은 0입니다.
- next를 표시할 때 다음 다섯 블록을 얻어오기 위해 덱 두 개를 사용했습니다. 하나의 덱에서 앞에서부터 빼 다른 덱이 저장하고, 다시 먼저 뺀 블록부터 원래의 덱에 넣는 방식입니다.
- 오른쪽 박스의 제일 윗 줄에는 현재 방해 요소를 나타냅니다. 키 변경, 블록 회전, 텍스쳐 변경 순으로 출력하는 코드가 표시되어 있습니다.

3. 블록 시각화와 SRS 시스템
- 블록 시각화는 ```const int block[][][][]```로 이루어졌습니다. ```block[type][rotation]```은 type번째 블록의 rotation번째 회전 상태에서 중심 위치에 대한 네 개의 정사각형의 상대 위치를 저장하는 이차원 배열입니다.
- SRS 회전 시스템은 srs와 srsI로 구현되었습니다. 각각은 삼차원 배열로 첫 번째 인자는 회전 상태, 두 번째 인자는 시계 또는 반시계의 방향, 세 번째 인자는 다섯 번의 순차적인 월 킥 이동을 나타냅니다.

4. 키 입력
- 키 입력은 ```keybind(int, int, int, int)``` 함수에서 이루어졌습니다. ```_kbhit()```으로 키 입력을 감지하고 ```_getch()```로 키를 가져왔습니다.
- 키 입력에 대해 현재 키 바인드에 따라 알맞은 함수를 호출합니다. 그러면 그 함수는 현재 블록의 상태를 나타내는 blockinfo를 받아 동작 이후의 blockinfo를 리턴합니다. ```keybind``` 함수는 이 blockinfo를 다시 리턴합니다.
- 키 입력에서는 바닥에 닿아도 일정 시간 이내에 키 입력이 있을 때 움직일 수 있도록 timer 변수를 리셋합니다. 하드 드랍 움직일 수 없어야 하기 때문에 ```isharddrop```로 관리합니다.
- 홀드도 한 번만 할 수 있또록 해야 합니다. ```ishold```로 관리합니다.

5. 메인 로직
- 먼저 시작 전 ```doubleConsole```을 초기화하고 block의 리스트를 생성합니다. 또한 다양한 방해 요소를 disable합니다.
- 바깥쪽 반복문은 블록이 한 번 설치 될 때마다 돌아옵니다. 즉, ```getnextblock()```로 블록을 얻어 오고 그 블록의 처음 위치를 정하고, ishold, isharddrop 등의 변수를 초기화 한 후 안쪽 반복문으로 이동합니다.
- 안쪽 반복문은 쉬지 않고 계속 반복됩니다. 다음은 안쪽 반복문에서 일어나는 로직입니다:
  - 출력을 위해 버퍼를 뒤집고 블록이 이동할 수 있으므로 이전의 블록을 보드에서 숨깁니다.
  - 중력에 의한 아래 방향 이동을 진행합니다. 주기는 500ms로 시작하여 피버 발동시마다 10ms씩 감소합니다.
  - 한 틱(1000/FPS)가 지날 때마다 피버를 계산합니다. 피버 시간 중에는 계산하지 않도록 하는 처리가 진행됩니다.
  - 바닥과의 충돌을 감지합니다. 소프트 드랍의 주기 만큼의 시간동안 입력이 감지되지 않는다면 블록을 놓기 위해 안쪽 반복문을 탈출합니다. 하드 드랍의 경우 무조건 탈출합니다.
  - 고스트 블록을 생성합니다. 하드 드랍시 나중 위치를 불러와 위치를 찾고 ```addghost```로 추가합니다.
  - 이후 ```summon```으로 현재 블록을 보두에 추가합니다.
  - ```printboard()```로 랜더링을 진행합니다.
  - ```delghost```로 고스트를 숨깁니다.

6. 보드 업데이트
- ```summon(int, int, int, int)```는 blockinfo를 받아 ```board[][]``` 배열을 변경합니다. blockinfo에 해당하는 곳에 블록을 추가합니다. 이 배열에 적힌 값대로 ```printboard()``` 함수에서 랜더링됩니다.
- ```dispawn(int, int, int, int)```도 blockinfo를 받아 ```board[][]``` 배열을 변경합니다. blockinfo에 해당하는 곳에 있던 블록을 제거합니다.
- ```addghost(int, int, int, int)```는 blockinfo를 받아 ```ghost[][]``` 배열을 변경합니다. blockinfo에 해당하는 곳에 고스트 블록을 추가합니다. 이 배열에 적힌 값대로 ```printboard()``` 함수에서 랜더링됩니다.
- ```delhost(int, int, int, int)```는 blockinfo를 받아 ```ghost[][]``` 배열을 변경합니다. blockinfo에 해당하는 곳에 고스트 블록을 제거합니다. 이 배열에 적힌 값대로 ```printboard()``` 함수에서 랜더링됩니다.
- ```getnextblock()```은 다음 블록을 리턴합니다. 만약 7개보다 적게 ```nextlist```에 남아 있다면 ```sevenbag()``` 함수에서 7-bag으로 일곱 개의 블록을 추가합니다. 이 함수에서 꽉 찬 줄을 지우는 ```remove_line()```을 호출하고, 게임 오버(초기 위치와 보드 사이의 충돌이 발생)를 판단합니다.

7. 피버 타임 로직
- ```fevercalculate()```에서는 빈 줄의 수를 계산하여 매 틱바다 증가할 피버의 양을 게산해 리턴합니다.
- ```feveractivate()```는 정해진 규칙에 따른 피버 타임 발생 시간을 정하고 랜덤으로 세 개 중 하나의 방해 요소를 활성화합니다. 10, 15, 20초의 지속 시간이 반복되며 한 세트가 돌면 2.5초씩 증가합니다.
- ```feverend```가 피버타임이 끝나는 시간을 저장합니다. 현재 시간이 이 시간보다 이후이고 피버타임이 진행 중이었다면 다시 방해 요소를 비활성화합니다.
- 다섯 번의 피버 타임을 지날 때마다 ```repeat``` 변수가 증가합니다. 피버가 ```repeat```배로 오르기 때문에 피버타임이 오는 속도가 반비례해서 빨라질 것입니다.

8. 보드 회전 로직
- ```boardrotation```을 추가하여 보드의 회전 상태를 변수로 씁니다. -1은 회전하지 않는 상태, 0~3은 각 회전 방향입니다.
- ```_cout, _wcout, printline, printrect```에 ```rotate=false```의 추가 파라미터를 넣습니다. 이 값이 ```true```이면 보드 회전에 영향을 받는다는 뜻입니다.
- 회전 중심 위치를 바탕으로 변환행렬을 계산합니다. ```cof[][][]``` 행렬이 그것입니다. 첫 번째 인자는 방향이고 각 cof[r]은 2 by 3 변환행렬입니다.
- ```rotate``` 파라미터가 추가된 함수에서 현재 상황에서 실제 회전 각도인 r을 계산합니다. rotate가 false이거나 blockrotation이 -1이면 r은 0이 될 것입니다.
- ```setrotate()``` 함수는 ```boardrotation``` 값을 0으로 바꿉니다. 활성화 되었다는 뜻입니다.
- ```resetrotate()``` 함수는 ```boardrotation``` 값을 -1로 바꿉니다. 비활성화 되었다는 뜻입니다.

9. 키 변경 로직
- 키의 셔플 상태를 ```key[]``` 배열에 저장합니다. 이는 섞였을 때의 순열이 저장된 것입니다. 즉, 처음 값은 {1, 2, 3, 4, 5, 6, 7}입니다.
- 키 변경은 ```shufflekey()```로 이루어집니다. 0 ~ 48 사이의 수를 뽑고 7로 나눈 몫과 나머지를 구해 각각에 해당하는 키를 교환합니다. 만약 두 값이 똑같다면 그 키를 쓰지 못하게 합니다.
- 키 변경 초기화는 ```resetkey()```로 이루어집니다. 처음 상태로 초기화합니다.

10. 텍스쳐 변경 로직
- 고스트 블록과 일반 블록의 텍스쳐를 ```wchar_t box```와 ```wchar_t gbox```에 저장힙니다.
- 보기 힘든 텍스쳐로 변경하는 것은 ```makesmall()```로 이루어집니다. 각각 점과 공백으로 바뀝니다.
- 초기화는 ```makelarge()```로 이루어집니다. 다시 원래의 사각형으로 바뀝니다.

11. 시간 및 랜덤
- 시간 계산은 ```chrono```를 사용합니다. ```gettime()```이 1970년 1월 1일부터 흐른 시간을 밀리초 단위로 반환합니다. 이에 따라 ```long long``` 자료형을 사용해야 합니다.
- 주기적인 반복은 마지막으로 실행된 시간을 저장하는 변수인 ```timer, softtimer, fevertimer``` 등을 활용하여 현재 시간과의 차를 계산함으로써 이루어집니다.
- 랜덤은 의사 난수인 ```mt19937```을 사용합니다. 104 ~ 110번 줄을 참고하세요.
