#include <windows.h>
#include <stdio.h>
#include <string.h>

#define ID_LIST 1
#define ID_EDIT 2
#define ID_ADD 3
#define ID_SAVE 4
#define ID_DELETE 5
#define ID_LOGIN 6
#define ID_REGISTER 7   // 🔥 NEW

HWND hList, hEdit, hPassword, hUsername;

char notes[100][512];
int noteCount = 0;

char currentUser[50];

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

// ---------------- REFRESH LIST ----------------
void RefreshList() {
    SendMessage(hList, LB_RESETCONTENT, 0, 0);

    int maxLen = 0;

    for (int i = 0; i < noteCount; i++) {
        char temp[600];
        sprintf(temp, "%d. %s", i + 1, notes[i]);
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)temp);

        int len = strlen(temp);
        if (len > maxLen) maxLen = len;
    }

    SendMessage(hList, LB_SETHORIZONTALEXTENT, maxLen * 7, 0);
}

// ---------------- CHECK LOGIN ----------------
int CheckLogin(char *username, char *password) {
    FILE *fp = fopen("users.txt", "r");

    if (!fp) return 0;

    char u[50], p[50];

    while (fscanf(fp, "%49s %49s", u, p) == 2) {
        if (strcmp(u, username) == 0 && strcmp(p, password) == 0) {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

// ---------------- REGISTER USER ----------------
void RegisterUser(char *username, char *password) {
    FILE *fp = fopen("users.txt", "a");

    if (!fp) {
        MessageBox(NULL, "Cannot open users.txt", "Error", MB_OK);
        return;
    }

    fprintf(fp, "%s %s\n", username, password);
    fclose(fp);

    MessageBox(NULL, "Registration Successful!", "Success", MB_OK);
}

// ---------------- LOAD NOTES ----------------
void LoadNotes() {
    char filename[100];
    sprintf(filename, "%s.txt", currentUser);

    FILE *fp = fopen(filename, "r");
    if (!fp) return;

    noteCount = 0;
    char buffer[512];

    while (fgets(buffer, sizeof(buffer), fp)) {
        buffer[strcspn(buffer, "\n")] = 0;
        strcpy(notes[noteCount++], buffer);
    }

    fclose(fp);
    RefreshList();
}

// ---------------- SAVE NOTES ----------------
void SaveNotes() {
    char filename[100];
    sprintf(filename, "%s.txt", currentUser);

    FILE *fp = fopen(filename, "w");
    if (!fp) return;

    for (int i = 0; i < noteCount; i++) {
        fprintf(fp, "%s\n", notes[i]);
    }

    fclose(fp);
}

// ---------------- MAIN ----------------
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR args, int ncmdshow) {

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProcedure;
    wc.hInstance = hInst;
    wc.lpszClassName = "MyWindowClass";

    RegisterClass(&wc);

    CreateWindow("MyWindowClass", "Multi-User Notes App",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, 560, 470,
        NULL, NULL, hInst, NULL);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// ---------------- WINDOW PROCEDURE ----------------
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

    switch (msg) {

    case WM_CREATE:

        CreateWindow("STATIC", "Username:",
            WS_VISIBLE | WS_CHILD,
            20, 20, 100, 20,
            hwnd, NULL, NULL, NULL);

        hUsername = CreateWindow("EDIT", "",
            WS_VISIBLE | WS_CHILD | WS_BORDER,
            20, 45, 200, 25,
            hwnd, NULL, NULL, NULL);

        CreateWindow("STATIC", "Password:",
            WS_VISIBLE | WS_CHILD,
            20, 80, 100, 20,
            hwnd, NULL, NULL, NULL);

        hPassword = CreateWindow("EDIT", "",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD,
            20, 105, 200, 25,
            hwnd, NULL, NULL, NULL);

        CreateWindow("BUTTON", "Login",
            WS_VISIBLE | WS_CHILD,
            20, 140, 80, 30,
            hwnd, (HMENU)ID_LOGIN, NULL, NULL);

        CreateWindow("BUTTON", "Register",
            WS_VISIBLE | WS_CHILD,
            120, 140, 100, 30,
            hwnd, (HMENU)ID_REGISTER, NULL, NULL);

        break;

    case WM_COMMAND:

        // ---------------- LOGIN ----------------
        if (LOWORD(wp) == ID_LOGIN) {

            char username[50], password[50];

            GetWindowText(hUsername, username, 50);
            GetWindowText(hPassword, password, 50);

            if (CheckLogin(username, password)) {

                strcpy(currentUser, username);

                hList = CreateWindow("LISTBOX", "",
                    WS_VISIBLE | WS_CHILD | WS_BORDER |
                    WS_VSCROLL | WS_HSCROLL | LBS_NOTIFY,
                    20, 190, 230, 250,
                    hwnd, (HMENU)ID_LIST, NULL, NULL);

                hEdit = CreateWindow("EDIT", "",
                    WS_VISIBLE | WS_CHILD | WS_BORDER |
                    ES_MULTILINE | WS_VSCROLL |
                    ES_AUTOVSCROLL | ES_WANTRETURN,
                    270, 190, 250, 160,
                    hwnd, (HMENU)ID_EDIT, NULL, NULL);

                CreateWindow("BUTTON", "Add",
                    WS_VISIBLE | WS_CHILD,
                    270, 360, 60, 30,
                    hwnd, (HMENU)ID_ADD, NULL, NULL);

                CreateWindow("BUTTON", "Save",
                    WS_VISIBLE | WS_CHILD,
                    340, 360, 60, 30,
                    hwnd, (HMENU)ID_SAVE, NULL, NULL);

                CreateWindow("BUTTON", "Delete",
                    WS_VISIBLE | WS_CHILD,
                    410, 360, 80, 30,
                    hwnd, (HMENU)ID_DELETE, NULL, NULL);

                LoadNotes();

            } else {
                MessageBox(hwnd, "Invalid Username or Password!", "Error", MB_OK);
            }
        }

        // ---------------- REGISTER ----------------
        if (LOWORD(wp) == ID_REGISTER) {

            char username[50], password[50];

            GetWindowText(hUsername, username, 50);
            GetWindowText(hPassword, password, 50);

            if (strlen(username) == 0 || strlen(password) == 0) {
                MessageBox(hwnd, "Enter username and password!", "Error", MB_OK);
                return 0;
            }

            RegisterUser(username, password);
        }

        // ---------------- ADD ----------------
        if (LOWORD(wp) == ID_ADD) {

            char buffer[512];
            GetWindowText(hEdit, buffer, 512);

            if (strlen(buffer) > 0 && noteCount < 100) {
                strcpy(notes[noteCount++], buffer);
                RefreshList();
                SetWindowText(hEdit, "");
            }
        }

        // ---------------- SAVE ----------------
        if (LOWORD(wp) == ID_SAVE) {

            int index = SendMessage(hList, LB_GETCURSEL, 0, 0);

            if (index != LB_ERR) {

                char buffer[512];
                GetWindowText(hEdit, buffer, 512);

                if (strlen(buffer) > 0) {
                    strcpy(notes[index], buffer);
                    RefreshList();
                }
            }

            SaveNotes();
            MessageBox(hwnd, "Saved!", "Info", MB_OK);
        }

        // ---------------- DELETE ----------------
        if (LOWORD(wp) == ID_DELETE) {

            int index = SendMessage(hList, LB_GETCURSEL, 0, 0);

            if (index != LB_ERR) {

                for (int i = index; i < noteCount - 1; i++) {
                    strcpy(notes[i], notes[i + 1]);
                }

                noteCount--;
                RefreshList();
                SetWindowText(hEdit, "");

            } else {
                MessageBox(hwnd, "Select a note first!", "Error", MB_OK);
            }
        }

        // ---------------- SELECT NOTE ----------------
        if (HIWORD(wp) == LBN_SELCHANGE) {

            int index = SendMessage(hList, LB_GETCURSEL, 0, 0);

            if (index != LB_ERR) {
                SetWindowText(hEdit, notes[index]);
            }
        }

        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}