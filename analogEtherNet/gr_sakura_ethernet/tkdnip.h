/*******************************************************************************
* GR-SAKURA LIBRARY LICENSE.  LIMITATION and DISCLAIMER of WARRANTY
*
* This software is supplied by Tokushu Denshi Kairo Inc. and is only intended 
* for use with Gadget Renesas projects. No other uses are authorized.
* This software is owned by Tokuden and is protected under all applicable laws,
* including copyright laws. Disclosure or redistribution to others this headder
* file and related library file alone is not allowed.
* You can redistribute the executable object file (only SREC or BIN file) which
* is generated as a result of compilation and link your program.
* Of cource the limitation of this library license does not effect to your code.
* THIS SOFTWARE IS PROVIDED "AS IS" AND TOKUDEN MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
* AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* Tokuden reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the
* http://rx.tokudenkairo.co.jp/
* (C) Copyright 2011-2012 Tokushu Denshi Kairo Inc.
*******************************************************************************/

#ifndef H_TKDNIP
#define H_TKDNIP
/**************************************************************************//**
 * @file    tkdnip.h
 * @brief   RX62N/RX63N用のTCP/IPプロトコルスタック。
 * @details このライブラリはTCP/IPレベルでの送受信を行う。Arduino互換ではなく独自実装。<BR>
 *          この関数群をArduino風にC++でラッパしたものが Ethernet.h で定義される TEthernet クラスと、 EthernetClient クラスと、 EthernetServer クラスである。ライブラリを使うにはリンカで -ltkdnip を指定すること。
 ******************************************************************************/

// 特電HAL
#include "tkdn_hal.h"

#ifndef BOOL
 #define BOOL  int
#endif

#ifdef __cplusplus
	extern "C" {
#endif

/*********************************************************************//**
 * @brief リンクの状態
 **********************************************************************/
typedef enum {
	LINK_DOWN         = 0, //!< @brief リンクダウン
	LINK_UP           = 1, //!< @brief リンクアップ
	LINK_DOWN_CHANGED = 2, //!< @brief リンクが切断された
	LINK_UP_CHANGED   = 3  //!< @brief リンクが接続された
} LINK_STATE;


/*********************************************************************//**
 * @brief DNSの応答のタイプ
 **********************************************************************/
typedef enum {
	DNS_NONAUTH,   //!< @brief ノンオーソリティ
	DNS_AUTH,      //!< @brief オーソリティ
	DNS_APPENDIX,  //!< @brief 追加情報
	DNS_UNDEF      //!< @brief 不明
} DNS_REPLY_TYPE;

/*********************************************************************//**
 * @brief DNSのレコードのタイプ
 **********************************************************************/
typedef enum {
	
	DNS_A     = 1,   //!< @brief IPv4 アドレス
	DNS_NS    = 2,   //!< @brief ネームサーバ
	DNS_CNAME = 5,   //!< @brief 別名
	DNS_PTR   = 12,  //!< @brief 逆引き
	DNS_MX    = 15,  //!< @brief メールサーバ
	DNS_AAAA  = 28,  //!< @brief IPv6アドレス
	DNS_ANY   = 255, //!< @brief 全部
} DNS_TYPE;

/*********************************************************************//**
 * @brief DNSのアドレスのタイプ
 **********************************************************************/
typedef enum {
	DNS_UNKN  = 0, //!< @brief 不明
	DNS_IN    = 1, //!< @brief インターネットアドレス
} DNS_CLASS;

/****************************************************************
* @brief   DNSの結果レコードが格納される構造体。
* @details dns_records_t 型に含まれるフィールドで用いられる
*          ユーザアプリで詳細なDNS情報を使う場合に使用する。
****************************************************************/
typedef struct dns_record_t
{
	DNS_REPLY_TYPE rtype;  //!< @brief DNS応答タイプ
	DNS_TYPE       type;   //!< @brief DNSのタイプ
	DNS_CLASS      aclass; //!< @brief アドレスクラス
	int            ttl;    //!< @brief 有効時間(秒)
	char           *name;  //!< @brief 名前
	unsigned char  *data;  //!< @brief データ
} dns_record_t;


/****************************************************************
* @brief   DNSの結果が格納される構造体 dns_record_t の配列を格納するための構造体
* @details tketh_nslookup_ex() の戻り値で使われる
* @note    この構造体が不要になったときには、 tketh_free_dns_records() で開放すること。
****************************************************************/
typedef struct dns_records_t
{
	int             num_of_records; //!< @brief DNSレコードの数
	dns_record_t  **record;         //!< @brief DNSレコード
} dns_records_t;

/****************************************************************
* @def        UDP_HANDLE_FUNC
* @brief   UDPを受信したときに呼び出されるユーザ関数
* @details この関数がTRUEを返すとユーザで処理したものと解釈され、システムでは処理されない。<BR>
*          この関数がFALSEを返すとシステムでは処理される。<BR>
* @note    この関数が呼び出されるとき、ポート番号とデータ長はリトルエンディアンになっている<BR>
****************************************************************/
typedef BOOL (*UDP_HANDLE_FUNC)(
	unsigned char dest_ip[4],
	unsigned char src_ip[4],
	unsigned short dest_port,
	unsigned short src_port,
	unsigned char *data, // UDPパケットのデータ部
	short datalen        // UDPパケットのデータの長さ(ヘッダは含まない)
);

//! @brief TCPの接続情報を示す構造体 内部構造は知る必要はないため隠蔽されている
typedef struct tcp_str tcp_t;
typedef struct tcp_server_str tcp_server_t;

//! @brief TCPの接続状態を表す定数
typedef enum {
      TCP_CLOSED,    //!< @brief CLOSED 接続は閉じている
      TCP_LISTEN,    //!< @brief LISTEN サーバが接続を待っている
      TCP_SYNRCVD,   //!< @brief SYN_RCVD SYNを受信した
      TCP_ESTAB,     //!< @brief ESTABLISHED 通信が確立している
      TCP_FIN_WAIT1, //!< @brief FIN_WAIT1 相手からのFINを待っている
      TCP_FIN_WAIT2, //!< @brief FIN_WAIT2 相手からのFINを待っている
      TCP_CLOSING,   //!< @brief CLOSING FINを送ったらFINを送り返された
      TCP_TIME_WAIT, //!< @brief TIME_WAIT クライアントからCLOSEされた後の待ち
      TCP_SYN_SENT,  //!< @brief SYN_SENT  SYNを送った。サーバからのACK待ち
      TCP_CLOSEWAIT, //!< @brief CLOSE_WAIT 
      TCP_LAST_ACK   //!< @brief LAST_ACK 
} TCP_STATE;

// クライアントの接続状態を示す構造体。 clients_stat_t のメンバとして使われる
typedef struct client_stat_t
{
	unsigned char  ipaddr[4];    // @brief クライアントIPアドレス
	unsigned short local_port;   // @brief ローカルポート番号
	unsigned short foreign_port; // @brief 相手先ポート番号
	BOOL           server;       // @brief 自分がサーバかどうか
	TCP_STATE      state;        // @brief TCPの状態
} client_stat_t;

// クライアントの接続状態を示す構造体のリスト。 tketh_get_client_stats() の結果として返される。
typedef struct clients_stat_t
{
	int             num_of_clients; // 接続しているクライアントの数
	client_stat_t  *clients;        // 接続クライアント情報
} clients_stat_t;

//! @brief TCPのイベントが起きた時にコールバックされる関数の型
typedef void (*FUNCTYPE_TCPEVENT)(tcp_t *client);

////////////////////////////////////////////////////////////////////////
//                         ユーザ公開関数
////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------
// 基本的な操作
//------------------------------------------------------------------------

/*********************************************************************//**
 * @brief		自分のMACアドレスを設定し、ライブラリを開始する
 * @param[in]   macaddr MACアドレス
 * @return		なし
 **********************************************************************/
void tketh_open(unsigned char *macaddr);

/*********************************************************************//**
 * @brief		イーサネットのリンクアップをチェックする。
 * @note        リンクが切断→接続に変わったら自動的にオートネゴーシエーションが実行される。
 * @return		 LINK_STATE 型の値。下記のとおり
 *              - LINK_DOWN         (0) : リンクはダウンしている
 *              - LINK_UP           (1) : リンクはアップしている
 *              - LINK_DOWN_CHANGED (2) : リンクは変化し、切断された
 *              - LINK_UP_CHANGED   (3) : リンクは変化し、接続された
 **********************************************************************/
LINK_STATE tketh_check_link();

/*********************************************************************//**
 * @brief       イーサネットの受信処理を進める
 * @details     この関数が呼び出されると、受信パケットの有無を調べ、
 *              パケットの内容に応じて受信処理を進める。<BR>
 *              受信エラーの解除も行われる。
 * @warning     この関数はメインのループの中で繰り返し呼び出さなければならない。
 * @return      なし
 **********************************************************************/
void tketh_process();

/*********************************************************************//**
 * @brief		デバッグレベルを設定する
 * @param		level
 *               - 0:なし
 *               - 1:パケットの概要を表示する
 *               - 2:パケットをダンプする
 * @return		なし
 **********************************************************************/
void tketh_debug(int level);

/*********************************************************************//**
 * @brief		自分の名前を設定する
 * @details		ここで設定したホスト名はDHCPで使われるので重要
 * @param		hostname ホスト名
 * @return		なし
 **********************************************************************/
void tketh_set_hostname(const char *hostname);

/*********************************************************************//**
 * @brief		自分の名前を調べる
 * @details		tketh_set_hostname() で設定した名前を返す
 * @return		自分のホスト名が格納された文字列バッファへのポインタ
 * @note		返された文字列はライブラリ内部でmallocして複製したもの
 * @warning		返された文字列をfreeしてはならない
 **********************************************************************/
char *tketh_get_hostname(void);

/*********************************************************************//**
 * @brief		MACアドレスを変更する
 * @param		macaddr 新しいMACアドレス
 * @return		なし
 **********************************************************************/
void tketh_change_macaddr(const unsigned char macaddr[6]);

/*********************************************************************//**
 * @brief		自分のMACアドレスを調べる
 * @param[out]	macaddr MACアドレスを格納するバッファへのポインタ
 * @return		なし
 * @note		バッファを確保するのはユーザルーチンで行うこと
 * @warning		戻り値用に6バイト以上の配列を確保しておくこと
 **********************************************************************/
void tketh_get_macaddr(unsigned char macaddr[6]);

//------------------------------------------------------------------------
// IPアドレス等の設定や取得
//------------------------------------------------------------------------

/*********************************************************************//**
 * @brief		DHCPを実行して、IPアドレス、GW、DNSサーバを設定する
 * @return		成功するとTRUEを返す。失敗するとFALSEを返す。
 **********************************************************************/
BOOL tketh_dhcpc(void);

/*********************************************************************//**
 * @brief		自分のIPアドレスを設定する
 * @details		IPアドレスとサブネットマスクを指定する。
 * @param[in]	ipaddr IPアドレスが格納された4バイトの配列
 * @param[in]	mask サブネットマスクが格納された4バイトの配列
 **********************************************************************/
void tketh_set_myipaddr(unsigned char ipaddr[4],unsigned char mask[4]);

/*********************************************************************//**
 * @brief		自分のIPアドレスを取得する
 * @details		DHCP等で設定されたIPアドレスを調べるときに使う
 * @param[out]	ipaddr IPアドレスを格納する4バイトの配列
 * @param[out]	mask サブネットマスクを格納する4バイトの配列
 * @note        この関数はメモリの確保は行わない。配列の実体を確保するのはユーザルーチンで行うこと
 * @return		なし
 **********************************************************************/
void tketh_get_myipaddr(unsigned char ipaddr[4],unsigned char mask[4]);

/*********************************************************************//**
 * @brief		デフォルトゲートウェイを設定する
 * @param[in]	ipaddr デフォルトゲートウェイのIPアドレス
 * @return		なし
 **********************************************************************/
void tketh_set_gateway(unsigned char ipaddr[4]);

/*********************************************************************//**
 * @brief		設定されているデフォルトゲートウェイのアドレスを取得する
 * @details		DHCP等で設定されたデフォルトゲートウェイアドレスを調べるときに使う
 * @param[out]	ipaddr デフォルトゲートウェイのIPアドレスを格納する4バイトの配列
 * @note        この関数はメモリの確保は行わない。配列の実体を確保するのはユーザルーチンで行うこと
 * @return		なし
 **********************************************************************/
void tketh_get_gateway(unsigned char ipaddr[4]);

/*********************************************************************//**
 * @brief		DNSサーバのアドレスを設定する
 * @param[in]	ipaddr DNSサーバのIPアドレス
 * @return		なし
 **********************************************************************/
void tketh_set_nameserver(unsigned char ipaddr[4]);

/*********************************************************************//**
 * @brief		設定されているDNSサーバのアドレスを取得する
 * @details		DHCP等で設定されたDNSサーバのアドレスを調べるときに使う
 * @param[out]	ipaddr DNSサーバのIPアドレスを格納する4バイトの配列
 * @note        この関数はメモリの確保は行わない。配列の実体を確保するのはユーザルーチンで行うこと
 * @return		なし
 **********************************************************************/
void tketh_get_nameserver(unsigned char ipaddr[4]);

/*********************************************************************//**
 * @brief		ARPのテーブルを表示する
 * @return		なし
 * @note		デフォルトのシリアルポートに表示する
 **********************************************************************/
void tketh_arp_table_show();

//------------------------------------------------------------------------
// 低レベルなパケットの送受信
//------------------------------------------------------------------------

/*********************************************************************//**
 * @brief		UDPを送信する
 * @param[in]	tgtaddr 相手先のIPアドレス
 * @param[in]	dest_port 送信先ポート番号
 * @param[in]	src_port 送信元ポート番号
 * @param[in]	buf 送信したいデータが格納されたバッファ
 * @param[in]	len 送信したいデータ長
 * @return		成功したらTRUE、失敗したらFALSEを返す
 * @note V1.07から、送信元ポート番号を指定できるようにしたため、パラメータ書式が変更になった
 **********************************************************************/
BOOL tketh_send_udp(unsigned char tgtaddr[4],unsigned short dest_port,unsigned short src_port,unsigned char *buf,int len);

/*********************************************************************//**
 * @brief       UDPを受信したときにコールバックされる関数を登録する
 * @param[in]   func UDPを受信したときにコールバックされる関数
 * @note        登録される関数は processPackets() の中から呼び出される。したがって、processPackets()を繰り返し呼び出していないと受信処理は行われない。
 **********************************************************************/
void tketh_regist_udp_handler(UDP_HANDLE_FUNC func);

//------------------------------------------------------------------------
// PING関係のプロトコル
//------------------------------------------------------------------------

/*********************************************************************//**
 * @brief		PINGを送信する
 * @param[in]   ipaddr Pingを送りたいホストのIPアドレス
 * @param[in]   buf Pingのデータ部に乗せるデータ。必要なければNULLでよい。
 * @param       len データ部の長さ。必要なければ0を指定してよい。
 * @return		成功したらTRUE、失敗したらFALSEを返す。
 **********************************************************************/
BOOL tketh_send_ping(unsigned char ipaddr[4],unsigned char *buf,int len);

/*********************************************************************//**
 * @brief		PINGが実行中かどうか調べる
 * @details		tketh_send_ping() で送信したPingが実行中かどうか調べる
 * @return		実行中ならTRUE、実行中でなければFALSEを返す。
 **********************************************************************/
BOOL tketh_is_ping_running();

/*********************************************************************//**
 * @brief		PINGの実行をキャンセルする
 * @details		tketh_send_ping() で開始したPingを中断する。
 * @return		なし
 **********************************************************************/
void tketh_cancel_ping();

/*********************************************************************//**
 * @brief		最後にPINGを送った時刻をus単位で返す
 * @return		Pingを最後に送った時間。システム起動時からの時間がms単位で返される。
 **********************************************************************/
unsigned long tketh_ping_starttime();

//------------------------------------------------------------------------
// 名前解決
//------------------------------------------------------------------------

/*********************************************************************//**
 * @brief		DNSサーバにアクセスして名前解決を行う
 * @param[in]	hostname 調べたいホスト名
 * @param[out]	ipaddr IPアドレス
 * @return		成功したらTRUE、失敗したらFALSEを返す
 **********************************************************************/
BOOL tketh_nslookup(char *hostname,unsigned char ipaddr[4]);

/*********************************************************************//**
 * @brief		DNSサーバにアクセスして名前解決を行い、すべてのレコードを得る
 * @details		dns_records_t *型のオブジェクトを返し、すべてのレコードを知ることができるようになっている。
 * @param[in]	hostname 調べたいホスト名
 * @return		結果が格納された dns_records_t *型の構造体へのポインタ
 * @note		ユーザルーチンはこの関数の戻り値は tketh_free_dns_records() 関数で開放しなければならない。
 * @warning		1500バイト以上の結果が返されたときには最後まで受信できない
 **********************************************************************/
dns_records_t *tketh_nslookup_ex(char *hostname);

/*********************************************************************//**
 * @brief		DNS結果構造体を開放する
 * @details		tketh_nslookup_ex()で得られた結果のオブジェクトを開放する。
 * @param[in]	dnsrec tketh_nslookup_ex() で得られたオブジェクト。
 * @return		なし
 **********************************************************************/
void tketh_free_dns_records(dns_records_t *dnsrec);

/*********************************************************************//**
 * @brief		DNSサーバにアクセスしてIPアドレスから名前を調べる
 * @param[in]	ipaddr 調べたいIPアドレス
 * @param[out]	hostname 変換されたホスト名が入るバッファ
 * @param[in]	hostname_maxlen バッファのサイズ
 * @return		成功したらTRUE、失敗したらFALSEを返す
 * @note        この関数は実装されていない
 **********************************************************************/
BOOL tketh_ip_to_hostname(unsigned char ipaddr[4],char *hostname,int hostname_maxlen);

//------------------------------------------------------------------------
// TCPクライアント
//------------------------------------------------------------------------

/*********************************************************************//**
 * @brief		TCPでサーバに接続する
 * @param[in]   ipaddr 接続したいサーバのIPアドレス
 * @param[in]   dest_port 接続したいポート番号
 * @return		成功したら tcp_t 型のオブジェクトを返す。失敗したらNULLを返す。
 **********************************************************************/
tcp_t *tketh_tcp_connect(unsigned char ipaddr[4],unsigned short dest_port);

/*********************************************************************//**
 * @brief		コネクションが接続中かどうかを調べる
 * @param[in]	info tketh_tcp_connect() で返されたオブジェクト
 * @return		接続中ならばTRUEを返す。切断されていればFALSEを返す。
 **********************************************************************/
BOOL tketh_tcp_is_connected(tcp_t *info);

/*********************************************************************//**
 * @brief		コネクションの穏やかな終了
 * @details		接続先のサーバにFINを送ることでコネクションを終了する
 * @param[in]	info tketh_tcp_connect() で返されたオブジェクト
 * @note		この関数はFINを送った後、相手からFINやACKが返るのを待たずに即座に戻る
 * @todo		FINタイマなどを実装するべき
 **********************************************************************/
void tketh_tcp_fin(tcp_t *info);

/*********************************************************************//**
 * @brief		コネクションの状態を文字列で返す
 * @param[in]	info tketh_tcp_connect() で返されたオブジェクト
 * @return		コネクションの状態を表す文字列
 * @note		この関数の戻り値はライブラリ内でstaticに確保された領域を指している。ユーザがfreeしてはならない。
 * @warning		この関数は実装されていない
 **********************************************************************/
char *tketh_tcp_state(tcp_t *info);

/*********************************************************************//**
 * @brief		TCPの接続先にデータを送信する
 * @param[in]	info tketh_tcp_connect() で返されたオブジェクト
 * @param[in]	data 送信したいデータが格納されたバッファ
 * @param[in]	len 送信したいデータ長
 **********************************************************************/
void tketh_tcp_senddata(tcp_t *info,unsigned char *data,int len);

/*********************************************************************//**
 * @brief		受信バッファに溜まっているデータの量を調べる
 * @details		TCPのパケットを受信すると受信バッファに格納される。その受信バッファ内のデータ数を返す。
 * @param		info tketh_tcp_connect() で返されたオブジェクト
 * @return		受信バッファ内のデータ数 (バイト単位)
 **********************************************************************/
int tketh_tcp_recvcount(tcp_t *info);

/*********************************************************************//**
 * @brief		受信バッファからデータを取り出す
 * @param[in]	info tketh_tcp_connect() で返されたオブジェクト
 * @param[out]	data 読み出したデータが格納されるバッファ。バッファは十分なサイズのメモリを確保しておくこと。
 * @param[in]	len 読み出したい長さ。
 * @return		受信したデータ数。データがなければ0を返す。
 **********************************************************************/
int tketh_tcp_recvdata(tcp_t *info,unsigned char *data,int len);

//------------------------------------------------------------------------
// TCPサーバ ※現在はセッションを１つしか張れない
//------------------------------------------------------------------------

/*********************************************************************//**
 * @brief		TCP接続を待ちうけたいポート番号をセットする
 * @details		この関数はあくまでも準備をするだけ。実際にLISTENステートには入れない。
 * @param[in]	port 接続を待ちうけるポート番号
 * @param[in]	func TCPのイベントが発生したときにコールバックされる関数 (この機能は未実装)
 * @return		TCPの接続情報を示すオブジェクトが返される
 * @note		この関数はあくまでもポート番号を指示するだけで、実際にはLISTEN状態にはならない
 **********************************************************************/
tcp_t *tketh_tcp_open_server(unsigned short port,FUNCTYPE_TCPEVENT func);

/*********************************************************************//**
 * @brief		サーバの受信開始
 * @details		サーバのTCPステートをLISTEN状態になる
 * @param[in]	info tketh_tcp_open_server() 関数で返されたオブジェクト
 * @return		成功したらTRUEを返す。失敗したらFALSEを返す。
 **********************************************************************/
BOOL tketh_tcp_listen(tcp_t *info);

/*********************************************************************//**
 * @brief		接続先のIPアドレスを調べる
 * @param[in]	info tketh_tcp_open_server() 関数で返されたオブジェクト
 * @return		接続先のIPアドレスが入った配列
 * @note		この関数の戻り値はライブラリ内でstaticに確保された領域なので、ユーザアプリでfreeをしてはいけない。
 **********************************************************************/
unsigned char *tketh_tcp_target_addr(tcp_t *info);

/*********************************************************************//**
 * @brief		サーバの待ち受け状態を終了する
 * @details		サーバのLISTEN状態を解除して、IDLE状態に戻す。この引数で指定した構造体へのオブジェクト info はもう使えない。
 * @param[in]	info tketh_tcp_open_server() 関数で返されたオブジェクト
 * @return		なし
 **********************************************************************/
void tketh_tcp_server_terminate(tcp_t *info);

//------------------------------------------------------------------------
// コネクション管理
//------------------------------------------------------------------------

/*********************************************************************//**
 * @brief		クライアントの状態を調べる
 * @details		本ライブラリは、サーバ・クライアント合わせて最大16個のセッションを張ることができる。その1つ1つのセッションに関する情報を返す。
 * @return		clients_stat_t *型オブジェクトへのハンドル
 **********************************************************************/
clients_stat_t *tketh_get_client_stats();

#ifdef __cplusplus
	}
#endif

#endif
