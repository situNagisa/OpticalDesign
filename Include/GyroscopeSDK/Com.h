#pragma once

#include "NGS/NGS.h"

class COM {
public:
	~COM() {
		if (_windowsData)
			ngs::Delete(_windowsData);
	}
	/**
	 * \brief 发送串口消息
	 *
	 * \param message 消息
	 * \param length 长度
	 */
	void Send(ngs::byte_ptr_cst message, size_t length);

	bool Receive(ngs::byte_ref data);
	/**
	 * \brief 设置波特率
	 *
	 * \param rate 波特率
	 */
	void SetBaudRate(ngs::uint32 rate);
	/**
	 * \brief 打开串口设备
	 *
	 * \param port 端口号
	 * \param rate 波特率
	 * \param receiveDataCallback 接收数据回调
	 * \return 是否成功打开串口设备
	 */
	bool Open(ngs::uint32 port, ngs::uint32 rate);
	/**
	 * \brief 关闭串口设备
	 *
	 */
	void Close();

	bool IsOpened()const { return _isOpen; }
private:

private:
	bool _isOpen = false;
	//windows数据，这样处理的话可以避免把windows API这坨屎暴露给外部
	ngs::void_ptr _windowsData = nullptr;
};
