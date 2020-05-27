#include <iostream>
#include<cstdio>
#include <pangolin/pangolin.h>
#include<fstream>
#include<cmath>
#include<queue>
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <sstream>
#include <geometry_msgs/PoseStamped.h>
#include <tf/tf.h>
#include <tf/transform_datatypes.h>
const float pi = 3.14159265;
using namespace std;
typedef pair<int, int> pii;
const int N = 200000;
const int max_z = 11.0;
const int max_x = 9.0;
const int delta =200;
float pt[N][3];
int cnt[max_x * 2 + 7][max_z * 2 + 7];
int dis[max_x * 2 + 7][max_z * 2 + 7];
pii last[max_x * 2 + 7][max_z * 2 + 7];
int n;
int dx[8] = {0, 0, 1, 1, 1, -1, -1, -1};
int dy[8] = {1, -1, 0, -1, 1, 0, -1, 1};
pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrix(1024,768,500,500,512,389,0.1,1000),
        //对应的是gluLookAt,摄像机位置,参考点位置,up vector(上向量)
        // pangolin::ModelViewLookAt(0,-0.7,-1.8, 0,0,0,0.0,-1.0, 0.0)
        pangolin::ModelViewLookAt(0,-3,0, 0,0,0,0.0,-1.0, 0.1)
);
// Define Projection and initial ModelView matrix


// Create Interactive View in window
pangolin::Handler3D handler(s_cam);
void rotate_aa(float x1, float y1, float alpha, float &x2, float &y2) {
    x2 = x1 * cos(alpha) + y1 * sin(alpha);
    y2 = -x1 * sin(alpha) + y1 * cos(alpha);
    y2 += 2.5;
    x2 += 1.5;
}
void calc_cnt() {
    for(int i = 1; i <= n; i++) {
        int x = ceil(pt[i][0] * 2), z = ceil(pt[i][2] * 2);
        if(x < 1 || x > max_x * 2 || z < 1 || z > max_z * 2) continue;
        cnt[x][z] ++;
    }
}
void precal() {
    cnt[5][1] = 0;
    for(int i = 1; i <= 6; i++) cnt[i][2] = delta + 1;
}
void bfs(int nowx, int nowy, int sx, int sy) {
    cout << "BFS()" << endl;
    queue<pii> q;
    q.push(make_pair(nowx, nowy));
    for(int i = 1; i <= max_x * 2; i++) for(int j = 1; j <= max_z * 2; j++) dis[i][j] = -1;
    dis[nowx][nowy] = 0;
    while(!q.empty()) {
        pii e = q.front();
        q.pop();
        int x = e.first, y = e.second;
        cout << "(" << x << ", " << y << ")"<< endl;
        for(int v = 0; v < 8; v++) {
            int xx = x + dx[v], yy = y + dy[v];
            if(xx < 1 || xx > 2 * max_x || yy < 1 || yy >2 * max_z || dis[xx][yy] >= 0) continue;
            //cout << xx << " " << yy << " " << cnt[xx][yy] << endl;
            if(cnt[xx][yy] > delta) continue;
            dis[xx][yy] = dis[x][y] + 1;
            last[xx][yy] = e;
            q.push(make_pair(xx, yy));
        }
    }
    cout << "Distance ...." << endl;
    cout << dis[sx][sy];
}
pair<float, float> grid2w(int x, int y) {
    float a = x, b = y;
    return make_pair(a / 2 - 0.28, b / 2 - 0.25);
}
void drawroute(int x, int y, int sx, int sy) {
    if (dis[sx][sy] < 0) return;
    glPointSize(20);
    glColor3f(1, 0, 0);
    glBegin(GL_LINE_STRIP);
    while(true) {
        pair<float, float> e = grid2w(sx, sy);
        glVertex3f(e.first, 0, e.second);
        if(x == sx && y == sy) break;
        pii la = last[sx][sy];
        sx = la.first, sy = la.second;
    }
    glEnd();
}
void drawpoints(int n) {
    glPointSize(2);
    glBegin(GL_POINTS);
    glColor3f(0.5,0.5,0.5);
    for(int i = 1; i <= n; i++)   glVertex3f(pt[i][0], 0, pt[i][2]);
    glEnd();

    // draw & limit
    /*
    glColor3f(1,0,0);
    glPointSize(5);
    glBegin(GL_LINE_LOOP);
    float minx = 0, maxx =9, minz = 0, maxz =11;
    glVertex3f(maxx, 0, maxz);
    glVertex3f(maxx, 0, minz);
    glVertex3f(minx, 0, minz);
    glVertex3f(minx, 0, maxz);
    glEnd();
    */

    // draw lines
    /*
    glBegin(GL_LINES);
    for(float i = 0; i <= max_x; i += 0.5) glVertex3f(i, 0, 0), glVertex3f(i, 0, max_z);
    for(float i = 0; i <= max_z; i += 0.5) glVertex3f(0, 0, i), glVertex3f(max_x, 0, i);
    glEnd();
    */

    // draw where cnt > delta
    /*
    glBegin(GL_POINTS);
    glPointSize(10);
    glColor3f(1,0,0);
    for(int i = 1; i <= max_x * 2; i++) for(int j = 1; j <= max_z * 2; j++) if(cnt[i][j] > delta){
        float x = i, y = j;
        glVertex3f(x / 2 - 0.25, 0, y / 2 - 0.25);
    }
    glEnd(); */
}
void drawlocation(float x, float y, float z) {
    glPointSize(10);
    glBegin(GL_POINTS);
    glColor3f(1,0,0);
    float xx, zz;
    rotate_aa(x, z, pi / 180 * 30, xx, zz);
    glVertex3f(xx, 0, zz);
    glEnd();
}
void calc(float x, float y, float z)  {
        pangolin::Var<bool> showroute("menu.ShowRoute",false,true);
        pangolin::Var<bool> showlocation("menu.Show current location",false,true);
        pangolin::Var<int> menu_x("menu.Dest X coordinates",11,1,20);
        pangolin::Var<int> menu_z("menu.Dest Z coordinates",15,1,20);
        pangolin::View &d_cam = pangolin::CreateDisplay().SetBounds(0.0, 1.0, pangolin::Attach::Pix(175), 1.0, -1024.0f/768.0f)
                .SetHandler(&handler);
        // Clear screen and activate view to render into
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        d_cam.Activate(s_cam);
        // pangolin::CreatePanel("menu").SetBounds(0.0,1.0,0.0,0.2);
        //第一个参数为按钮的名字，第二个为默认状态，第三个为是否有选择框
        // pangolin::Var<bool> menu("menu.test",false,true);
        // Render OpenGL Cube

        //pangolin::glDrawColouredCube();
        //坐标轴的创建
        pangolin::glDrawAxis(60);
        glClearColor(1.0f,1.0f,1.0f,1.0f);
        //点的创建
        drawpoints(n);
        //drawmap();
        if(showlocation)
            drawlocation(x, y, z);
        if(showroute)
            //drawroute(1, 1, 11, 15);
            drawroute(1, 1, menu_x, menu_z);
        // Swap frames and Process Events
        pangolin::FinishFrame();
}
void chatterCallback(geometry_msgs::PoseStamped posestamp)
{
    float x = posestamp.pose.position.x;
    float y = posestamp.pose.position.y;
    float z = posestamp.pose.position.z;
    calc(x, y, z);
    //printf("Pose: (%.5lf, %.5lf, %.5lf)\n", posestamp.pose.position.x, posestamp.pose.position.y, posestamp.pose.position.z);
}
int main(int argc, char **argv)
{
    ifstream f("MapPoint.txt");
    if(f.fail())  {
        cout << "Fail" << endl;
        return 0;
    }
    f >> n;
    for(int i = 1; i <= n; i++) f >> pt[i][0] >> pt[i][1] >> pt[i][2];
    for(int i = 1; i <= n; i++) {
        float x, z;
        rotate_aa(pt[i][0], pt[i][2], pi / 180 * 30, x, z);
        pt[i][0] = x, pt[i][1] = 0, pt[i][2] = z;
    }
    calc_cnt();
    precal();
    float mi[3],ma[3];
    for(int i = 0; i < 3; i++) mi[i] = 100, ma[i] = -100;
    for(int i = 1; i <= n; i++) for(int j = 0; j < 3; j++) {
            mi[j] = min(mi[j], pt[i][j]);
            ma[j] = max(ma[j], pt[i][j]);
        }
    printf("Min:\n");
    for(int i = 0; i < 3; i++) printf("%.5f ", mi[i]);
    printf("\nMax:\n");
    for(int i = 0; i < 3; i++) printf("%.5f ", ma[i]);
    cout << endl;
    bfs(1, 1, 11, 15);
    //创建一个窗口
    pangolin::CreateWindowAndBind("Main",1024,768);
    //启动 深度测试
    pangolin::CreatePanel("menu").SetBounds(0.0,1.0,0.0,0.2);


    glEnable(GL_DEPTH_TEST);
    //while(!pangolin::ShouldQuit())
    ros::init(argc, argv, "listener");
    ros::NodeHandle n;
    ros::Subscriber sub = n.subscribe("ORB_SLAM/pose", 5, chatterCallback);
    ros::spin();
    f.close();
    return 0;
}
