#include <windows.h>
#include <string.h>
#include <array>
#include <algorithm>
#include <numeric>
#include <random>

//↓これでコンパイルして
//x86_64-w64-mingw32-g++ window.cpp -mwindows \
  -static-libgcc -static-libstdc++ -o HitAndBlow.exe



using namespace std;

enum {
    ID_EDIT1 = 1001,
    ID_EDIT2,
    ID_EDIT3,
    ID_EDIT4,

    ID_BUTTON_DECIDE    = 2001, //決定ボタン
    ID_LIST_HISTORY     = 2002, //過去の数値の履歴
};

static array<int, 4> secret; // 数値を格納する配列

//ランダムにsecretを生成
void GenerateSecret()
{
    array<int, 10> num;
    iota(num.begin(), num.end(), 0); // 0から9までの数値を生成
    random_device rd; // 乱数生成器
    mt19937 gen(rd()); // メルセンヌ・ツイスター法の乱数生成器
    shuffle(num.begin(), num.end(), gen); // 数値をランダムにシャッフル

    // 最初の4つの数値をsecretに格納
    for (int i = 0; i < 4; ++i) {
        secret[i] = num[i];
    }
}

//ヒットアンドブロー
//first:ヒットの数, second:ブローの数
pair<int, int> HitAndBlow(const array<int, 4>& guess)
{
    int hit = 0, blow = 0;
    bool usedSec[4] = {false, false, false, false}; // 位置を記録するための配列
    bool usedGuess[4] = {false, false, false, false}; // secretの位置を記録するための配列

    //Hit: 同じ位置に同じ数字がある場合
    for (int i = 0; i < 4; ++i)
    {
        if (guess[i] == secret[i]) {
        hit++;
        usedSec[i]   = true;
        usedGuess[i] = true;
        }
    }
    //Blow: 同じ数字があるが位置が異なる場合
    for(int i = 0; i < 4; ++i)
    {
        if (usedGuess[i]) continue;       // Hit 済みはスキップ
        for(int j = 0; j < 4; ++j)
        {
            if (usedSec[j]) continue;       // 既に使った secret もスキップ
            if (guess[i] == secret[j])
            {
                blow++;
                usedSec[j] = true;            // この secret[j] はもう数えない
                break;                        // guess[i] も次へ
            }
        }
    }
    return {hit, blow};
}

static HFONT hFONT = nullptr;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam) -> hInstance;
            const int editW     = 50;
            const int editH     = 50;
            const int spacing   = 50;  //ボックス座標x
            const int topMargin = 300;  //ボックス座標y

            /*フォントに関して*/
            //フォント指定
            hFONT = CreateFont
            (
                -24,                            // 高さ：負の値でポイント指定
                0,                              // 文字幅（自動）
                0, 0,                           // 文字傾き
                FW_NORMAL,                      // 太さ
                FALSE, FALSE, FALSE,
                SHIFTJIS_CHARSET,               // 文字セット
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY,
                DEFAULT_PITCH | FF_DONTCARE,
                "ＭＳ 明朝"                     // フォント名
            );

            /*フォントに関して*/

            /*----------テキストボックスを生成----------*/
            for(int i=0; i<4; i++)  //ウィンドウの左端回避で1から始めている
            {   
                HMENU ctrlId = (HMENU)(UINT_PTR)(ID_EDIT1 + i); //64bit ビルドでは、HMENU（ポインタ型）が 64bit、大きさの異なる int（32bit）を直接キャストすると警告が出る

                HWND hEdit = CreateWindowEx
                (
                    WS_EX_CLIENTEDGE,
                    "EDIT",                     //クラス名
                    "",                         //初期文字列
                    WS_CHILD | WS_VISIBLE |
                    WS_BORDER | ES_CENTER |
                    ES_NUMBER,                  //数字しかダメ
                    spacing + (i+1) * 1.7 * (editW + spacing),  //x
                    topMargin,                              //y
                    editW, editH,
                    hwnd,
                    ctrlId,                     //コントロールを連番で
                    hInst,
                    nullptr
                );
                //最大入力を1文字に制限
                SendMessage(hEdit, EM_SETLIMITTEXT, 1, 0);
            }
            /*----------テキストボックスを生成----------*/

            /*決定ボタン*/
            CreateWindow
            (
                "BUTTON",                                   // クラス名
                "GO",                                       // キャプション
                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                spacing + 2 * (editW + spacing),            // X位置
                (topMargin + 150),                          // Y位置
                500,                                        // 幅
                editH,                                      // 高さ
                hwnd,                                       // 親ウィンドウ
                (HMENU)(UINT_PTR)ID_BUTTON_DECIDE,          // コントロールID
                hInst,                                      // HINSTANCE
                nullptr                                     // 作成パラメータ
            );
            /*決定ボタン*/

            CreateWindowEx
            (
                WS_EX_CLIENTEDGE,
                "LISTBOX",
                nullptr,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
                150, 50,         //x, y
                700, 200,       //幅220,高さ150
                hwnd,
                (HMENU)(UINT_PTR)ID_LIST_HISTORY,
                hInst,
                nullptr
            );


            // 1) ループで EDIT1～EDIT4 に適用
            for(int id = ID_EDIT1; id <= ID_EDIT4 + 1; ++id) {
                HWND hEd = GetDlgItem(hwnd, id);
                SendMessage(hEd, WM_SETFONT, (WPARAM)hFONT, MAKELPARAM(TRUE,0));
            }

            // 2) Decide ボタンに適用
            HWND hBtn = GetDlgItem(hwnd, ID_BUTTON_DECIDE);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFONT, MAKELPARAM(TRUE,0));

            // 3) Listbox に適用
            HWND hList = GetDlgItem(hwnd, ID_LIST_HISTORY);
            SendMessage(hList, WM_SETFONT, (WPARAM)hFONT, MAKELPARAM(TRUE,0));

            GenerateSecret(); // ゲーム開始時にランダムな数字を生成

            return 0;
        }


        case WM_DESTROY:
            if (hFONT) {
                DeleteObject(hFONT);
                hFONT = nullptr;   // 二重解放防止
            }
            PostQuitMessage(0);
            return 0;

        case WM_COMMAND:
            //決定が押されたらここに保存
            if(LOWORD(wParam) == ID_BUTTON_DECIDE && HIWORD(wParam) == BN_CLICKED)
            {   
                /*入力取得・数値変換*/
                array<int, 4> guess;
                char buf[8];
                bool valid = true;
                for(int i=0; i<4; i++)
                {
                    GetDlgItemText(hwnd, ID_EDIT1 + i, buf, sizeof(buf));   //ここでエラーはく
                    //範囲外の場合は無効
                    if(buf[0] < '0' || buf[0] > '9')
                    {
                        valid = false;
                        break;
                    }
                    guess[i] = buf[0] - '0'; //文字を数値に変換
                }
                if(!valid)
                {
                    MessageBox(hwnd, "you should input 0~9", "Error", MB_OK | MB_ICONERROR);
                    return 0;
                }

                /*ヒットアンドブローの計算*/
                auto result = HitAndBlow(guess);
                int hit = result.first;
                int blow = result.second;

                /*リストに追加*/
                char entry[64];

                sprintf(entry, "Input: %d%d%d%d, Hit: %d, Blow: %d",
                    guess[0], guess[1], guess[2], guess[3], hit, blow);
                HWND hEdit = GetDlgItem(hwnd, ID_LIST_HISTORY);
                SendMessage(hEdit, LB_ADDSTRING, 0, (LPARAM)entry);
                int count = (int)SendMessage(hEdit, LB_GETCOUNT, 0, 0);
                SendMessage(hEdit, LB_SETTOPINDEX, count - 1, 0);

                /*正解したらリセット*/
                if(hit == 4)
                {
                    MessageBox(hwnd, "Congratulations!", "You got it", MB_OK | MB_ICONINFORMATION);
                    GenerateSecret(); // 新しい数字を生成
                    SendMessage(hEdit, LB_RESETCONTENT, 0, 0); // 履歴リストをクリア
                }
            }
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const char CLASS_NAME[] = "CppApp_Window";

    WNDCLASS wc = {};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "Hit_And_Blow",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        1000, 600, nullptr, nullptr, hInstance, nullptr
      //　↑　　↑　windowサイズ
    );

    if (hwnd == nullptr) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}