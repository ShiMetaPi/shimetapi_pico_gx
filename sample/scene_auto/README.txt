1、使用 sdk\sample\pq_bin 中的编译的程序pq_bin导入 sdk\sample\scene_auto 目录下的sensor图像效果bin文件
2、根据bin 文件命名，比如 xxx_linear_isp_vpss.bin
     eg:导入 sc485sl_linear_isp_vpss_ainr.bin
     cd  sdk/sample/pq_bin
     ./pq_bin 0 ../scene_auto/sc485sl/sc485sl_linear_isp_vpss_ainr.bin
3、导出调试图像效果bin文件时使用pqtool工具界面上的导出按钮或者使用pq_bin程序执行导出均可
