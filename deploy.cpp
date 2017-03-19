#include "deploy.h"
#include "utils.h"
#include <stdio.h>
#include <cstring>
#define MAX_CAPACITY 5000
#define MAX_PRICE 10000
//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
	//n:net l:link c:consumer s:server
//	网络节点数量 网络链路数量 消费节点数量
//	（空行）
//	视频内容服务器部署成本
//	（空行）
//	链路起始节点ID 链路终止节点ID 总带宽大小 单位网络租用费
//	…………….（如上链路信息若干行）
//	（空行）
//	消费节点ID 相连网络节点ID 视频带宽消耗需求
//	…………….（如上终端用户信息若干行）
//	（文件结束）
	int n_node_num,l_num,c_node_num;
	int s_price;
	sscanf(topo[0],"%d %d %d",&n_node_num,&l_num,&c_node_num);
	sscanf(topo[2],"%d",&s_price);


	int capacity_matrix[n_node_num][n_node_num]={0},price_matrix[n_node_num][n_node_num]={0},need_matrix[c_node_num][n_node_num]={0};
	int i,j,capacity,price,need;
	//capacity and price
    for(int i_line=4;i_line<l_num+4;i_line++){
		sscanf(topo[i_line],"%d %d %d %d",&i,&j,&capacity,&price);
		capacity_matrix[i][j]=capacity;
		price_matrix[i][j]=price;
	}
//    for(int i=0;i<n_node_num;i++){
//        for(int j=0;j<n_node_num;j++){
//            printf("%d\t",capacity_matrix[i][j]);
//        }
//        printf("\n");
//    }

    //need
	for(int i_line=l_num+5;i_line<l_num+c_node_num+5;i_line++){
		sscanf(topo[i_line],"%d %d %d",&i,&j,&need);
		need_matrix[i][j]=need;
	}
//    for(int i=0;i<c_node_num;i++){
//        for(int j=0;j<n_node_num;j++){
//            printf("%d\t",need_matrix[i][j]);
//        }
//        printf("\n");
//    }
    /****************************************************************************************************/
	//构造完整的图，增加源点和汇点,重新对节点编号：总源点s+网络节点+总汇点
	int node_num=1+n_node_num+1;
	int assist_capacity_matrix[node_num][node_num]={0};//用于整个算法，不可改变
	int assist_price_matrix[node_num][node_num]={0};//用于整个算法，不可改变

    //初始化
    //没有通路的两节点的容量和价格都为0,在计算时需要判断
	//1.源节点
	//遍历need_matrix,而不是capacity_assist_matrix
	for(int i=0;i<c_node_num;i++){
		for(int j=0;j<n_node_num;j++){
			if(need_matrix[i][j]!=0){
				assist_capacity_matrix[0][j+1]=need_matrix[i][j];//需有一个偏移，因为辅助矩阵增加了源节点和汇点
				assist_capacity_matrix[j+1][0]=need_matrix[i][j];
				assist_price_matrix[0][j+1]=0;
				assist_price_matrix[j+1][0]=0;
			}
		}
	}
	//2.网络节点
	for(int i=0;i<n_node_num;i++){
		for(int j=i+1;j<n_node_num;j++){
			assist_capacity_matrix[i+1][j+1]=capacity_matrix[i][j];
			assist_capacity_matrix[j+1][i+1]=capacity_matrix[i][j];
			assist_price_matrix[i+1][j+1]=price_matrix[i][j];
			assist_price_matrix[j+1][i+1]=price_matrix[i][j];
		}
	}
	//3.总汇点
    for(int j=0;j<n_node_num;j++){
        assist_capacity_matrix[node_num-1][j+1]=MAX_CAPACITY;
        assist_capacity_matrix[j+1][node_num-1]=MAX_CAPACITY;
        assist_price_matrix[node_num-1][j+1]=-1;
        assist_price_matrix[j+1][node_num-1]=-1;
    }

//    for(int i=0;i<node_num;i++){
//        for(int j=0;j<node_num;j++){
//            printf("%d\t",assist_capacity_matrix[i][j]);
//        }
//        printf("\n");
//    }
//    for(int i=0;i<node_num;i++){
//        for(int j=0;j<node_num;j++){
//            printf("%d\t",assist_price_matrix[i][j]);
//        }
//        printf("\n");
//    }

    //构造增量网络/残存网络
    int residual_capacity_matrix[node_num][node_num]={0};
    int residual_price_matrix[node_num][node_num] = {0};
    int flew_matrix[node_num][node_num]={0};//上三角矩阵
    int val_flew=0;//当前流量值
    int val;//期望的流量值
    for(int i=0;i<c_node_num;i++){
        for(int j=0;j<n_node_num;j++){
            val+=need_matrix[i][j];
        }
    }
    printf("%d,%d\n",val,val_flew);
/******************************************************************************************************************/
    while(true) {
        if (val_flew == val) {
            break;
        }
        for (int i = 0; i < node_num; i++) {
            for (int j = i + 1; j < node_num; j++) {
                if(assist_capacity_matrix[i][j]!=0){
                    if(assist_price_matrix[i][j]==-1){//有通路,但除去出节点为汇点的边
                        //(i,j)容量不变，为Max,价格为0
                        //(j,i)容量为flew,价格为+
                        residual_capacity_matrix[i][j] = assist_capacity_matrix[i][j];
                        residual_capacity_matrix[j][i] = flew_matrix[i][j];
                        if (flew_matrix[i][j] > 0) {
                            residual_price_matrix[i][j] = 0;
                            residual_price_matrix[j][i] = 0;
                        }else{
                            residual_price_matrix[i][j] =s_price;
                            residual_price_matrix[j][i]=-1;
                        }
                    }else{
                        residual_capacity_matrix[i][j] = assist_capacity_matrix[i][j] - flew_matrix[i][j];
                        residual_capacity_matrix[j][i] = flew_matrix[i][j];
                        residual_price_matrix[j][i] = -assist_price_matrix[j][i];
                        residual_price_matrix[i][j] = assist_price_matrix[i][j];
                    }
                }
            }
        }
//        for(int i=0;i<node_num;i++){
//            for(int j=0;j<node_num;j++){
//                printf("%d\t",residual_capacity_matrix[i][j]);
//            }
//            printf("\n");
//        }
//        for(int i=0;i<node_num;i++){
//            for(int j=0;j<node_num;j++){
//                printf("%d\t",residual_price_matrix[i][j]);
//            }
//            printf("\n");
//        }
        //在残存网络中找费用最小的路
        //Bellman-Ford算法
        int node_label_array[node_num] = {0};//节点标号数组,用于找最小费用路
        int parent[node_num] = {0};//用于存储最小费用路
        for (int i = 0; i < node_num; i++) {
            node_label_array[i] = MAX_PRICE;
            parent[i] = -1;
        }
        node_label_array[0] = 0;
        parent[0] = 0;//源点的父节点是自己
        for (int k = 0; k < node_num; k++) {
            for (int i = 0; i < node_num; i++) {//遍历每条边node_num*node_num
                for (int j = 0; j < node_num; j++) {
                    if (residual_capacity_matrix[i][j]!=0) {
                        if(node_label_array[j] > node_label_array[i] + residual_price_matrix[i][j]){
                            node_label_array[j] = node_label_array[i] + residual_price_matrix[i][j];
                            printf("(%d,%d):%d\n",i,j,node_label_array[j]);
                            parent[j] = i;
                        }
                    }
                }
            }
        }

        for(int i=0;i<node_num;i++){
            printf("%d\t",node_label_array[i]);
        }
        printf("\n");
        for(int i=0;i<node_num;i++){
            printf("%d\t",parent[i]);
        }

        if (node_label_array[node_num - 1 == MAX_PRICE]) {
//            while (true) {
                printf("残存网络不存在从源点到汇点的有向路");
//            }
        }

        for (int i = 0; i < node_num; i++) {
            for (int j = 0; j < node_num; j++) {
                if (node_label_array[j] > node_label_array[j] + residual_price_matrix[i][j]) {
//                    while (true) {
//                        printf("\n残存网络中出现负值环");
//                    }
                }
            }
        }
        break;
        int min_flew = MAX_CAPACITY;
        for (int i = node_num - 1; i > 0; i = parent[i]) {
            if (residual_capacity_matrix[parent[i]][i] < min_flew) {
                min_flew = residual_capacity_matrix[parent[i]][i];
            }
        }
        min_flew = min_flew < val - val_flew ? min_flew : val - val_flew;
        for (int i = node_num - 1; i > 0; i = parent[i]) {//沿着parent路增流
            flew_matrix[parent[i]][i] += min_flew;
            val_flew+=min_flew;
        }
    }
	// 需要输出的内容
	char * topo_file = (char *)"17\n\n0 8 0 20\n21 8 0 20\n9 11 1 13\n21 22 2 20\n23 22 2 8\n1 3 3 11\n24 3 3 17\n27 3 3 26\n24 3 3 10\n18 17 4 11\n1 19 5 26\n1 16 6 15\n15 13 7 13\n4 5 8 18\n2 25 9 15\n0 7 10 10\n23 24 11 23";

	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(topo_file, filename);

}
