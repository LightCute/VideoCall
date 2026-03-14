#include "widget.h"
#include "./ui_widget.h"
#include "utilities/log.h"
#include "socket.h"
#include <QRegularExpression>
Widget::Widget(WebSocket* ws, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_ws(ws) // 初始化指针
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_bt_connect2Server_clicked()
{
    if (m_ws == nullptr) {
        Log::error("[Widget] WebSocket pointer is null!");
        return;
    }

    // ========== 步骤1：读取并校验4位字母用户名 ==========
    QString username = ui->TxEd_bt_connect2Server->toPlainText(); // 去除首尾空格

    // 校验1：用户名非空
    if (username.isEmpty()) {
        Log::warn("[Widget] Username is empty!");
        return;
    }

    // 校验2：用户名长度必须为4
    if (username.length() != 4) {
        Log::warn("[Widget] Username length must be 4 characters! Current length: {}", username.length());
        return;
    }

    // 校验3：用户名必须全为字母（大小写均可，如需仅小写可加.toLower()）
    QRegularExpression letterRegex("^[A-Za-z]{4}$"); // 正则：仅4个字母
    if (!letterRegex.match(username).hasMatch()) {
        Log::warn("[Widget] Username must contain only 4 letters! Input: {}", username.toStdString());
        return;
    }

    // ========== 步骤2：读取服务器IP（可选：固定/用户输入） ==========
    // 方案A：服务器IP+端口固定（推荐，减少用户输入）
    const QString server_ip_port = "120.79.210.6:8000";

    // 方案B：用户输入IP+端口（如需此方案，新增QLineEdit：le_server_ip）
    // QString server_ip_port = ui->le_server_ip->text().trimmed();
    // if (server_ip_port.isEmpty()) {
    //     Log::warn("[Widget] Server IP:Port is empty!");
    //     return;
    // }

    // ========== 步骤3：拼接完整URL ==========
    QString qt_url = QString("ws://%1/%2").arg(server_ip_port).arg(username);

    // ========== 步骤4：转换并调用连接函数 ==========
    std::string std_url = qt_url.toStdString();
    std::string std_username = username.toStdString();
    m_ws->connect2Server(std_url, std_username);

    Log::info("[Widget] Connect to server with URL: {}", std_url);
}

void Widget::on_bt_call_clicked()
{
    if (m_ws == nullptr) {
        Log::error("[Widget] WebSocket pointer is null!");
        return;
    }

    // ========== 步骤1：读取并校验4位字母用户名 ==========
    QString peerName = ui->TxEd_bt_connect2Server->toPlainText(); // 去除首尾空格

    // 校验1：用户名非空
    if (peerName.isEmpty()) {
        Log::warn("[Widget] peerName is empty!");
        return;
    }

    // 校验2：用户名长度必须为4
    if (peerName.length() != 4) {
        Log::warn("[Widget] peerName length must be 4 characters! Current length: {}", peerName.length());
        return;
    }

    // 校验3：用户名必须全为字母（大小写均可，如需仅小写可加.toLower()）
    QRegularExpression letterRegex("^[A-Za-z]{4}$"); // 正则：仅4个字母
    if (!letterRegex.match(peerName).hasMatch()) {
        Log::warn("[Widget] peerName must contain only 4 letters! Input: {}", peerName.toStdString());
        return;
    }

    m_ws->connect2Peer(peerName.toStdString());
}

