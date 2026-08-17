#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>

using namespace std;

// 직선 형태의 벽을 맵에 그리는 헬퍼 함수
void drawLine(vector<vector<char>>& grid, int x, int y, int length, bool isHorizontal, int width, int height) {
    for (int i = 0; i < length; ++i) {
        int nx = x + (isHorizontal ? i : 0);
        int ny = y + (isHorizontal ? 0 : i);

        // 맵 테두리를 벗어나지 않도록 범위 체크
        if (nx >= 1 && nx < width - 1 && ny >= 1 && ny < height - 1) {
            grid[ny][nx] = '#';
        }
    }
}

int main() {
    // 맵 설정
    const int width = 400;
    const int height = 200;
    const int num_walls = 120;     // 생성할 벽의 개수
    const int min_len = 10;        // 벽의 최소 길이
    const int max_len = 30;        // 벽의 최대 길이
    const string output_filename = "RandomMap.txt";

    // 난수 생성기 초기화 (Mersenne Twister)
    random_device rd;
    mt19937 gen(rd());

    // 난수 범위 설정
    uniform_int_distribution<> distX(5, width - 35);
    uniform_int_distribution<> distY(5, height - 35);
    uniform_int_distribution<> distLen(min_len, max_len);
    uniform_int_distribution<> distType(0, 3); // 0: 가로, 1: 세로, 2: L자, 3: T자
    uniform_int_distribution<> distBool(0, 1);

    // 1. 빈 맵 생성 (모든 공간을 '.'으로 초기화)
    vector<vector<char>> grid(height, vector<char>(width, '.'));

    // 외곽 테두리 벽(#) 생성
    for (int x = 0; x < width; ++x) {
        grid[0][x] = '#';
        grid[height - 1][x] = '#';
    }
    for (int y = 0; y < height; ++y) {
        grid[y][0] = '#';
        grid[y][width - 1] = '#';
    }

    // 2. 무작위 벽(직선, L자, T자) 생성
    for (int i = 0; i < num_walls; ++i) {
        int x = distX(gen);
        int y = distY(gen);
        int length = distLen(gen);
        int wall_type = distType(gen);

        if (wall_type == 0) {
            // 가로 직선
            drawLine(grid, x, y, length, true, width, height);
        }
        else if (wall_type == 1) {
            // 세로 직선
            drawLine(grid, x, y, length, false, width, height);
        }
        else if (wall_type == 2) {
            // L자 모양
            drawLine(grid, x, y, length, true, width, height);
            int v_len = distLen(gen);
            int corner_x = distBool(gen) ? (x + length - 1) : x;
            drawLine(grid, corner_x, y, v_len, false, width, height);
        }
        else if (wall_type == 3) {
            // T자 모양
            drawLine(grid, x, y, length, true, width, height);
            int stem_len = distLen(gen);
            int mid_x = x + length / 2;
            drawLine(grid, mid_x, y, stem_len, false, width, height);
        }
    }

    //// 3. 엔티티(플레이어, 아이템) 배치 함수
    //uniform_int_distribution<> randX(2, width - 3);
    //uniform_int_distribution<> randY(2, height - 3);

    //auto placeEntity = [&](char entity) {
    //    while (true) {
    //        int rx = randX(gen);
    //        int ry = randY(gen);
    //        // 빈 공간('.')인 곳에만 배치
    //        if (grid[ry][rx] == '.') {
    //            grid[ry][rx] = entity;
    //            break;
    //        }
    //    }
    //    };

    //// 플레이어(P) 1개, 아이템(1) 3개, 아이템(2) 3개 랜덤 배치
    //placeEntity('P');
    //for (int i = 0; i < 3; ++i) placeEntity('1');
    //for (int i = 0; i < 3; ++i) placeEntity('2');

    // 4. 파일로 저장
    ofstream outFile(output_filename);
    if (outFile.is_open()) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                outFile << grid[y][x];
            }
            outFile << "\n";
        }
        outFile.close();
        cout << "성공적으로 " << width << "x" << height << " 크기의 맵을 생성하여 '" << output_filename << "'에 저장했습니다!\n";
    }
    else {
        cerr << "파일을 열 수 없습니다. 권한을 확인해주세요.\n";
    }

    return 0;
}