using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Net;
using System.Net.Sockets;

namespace UdpList
{
    class UdpSocketPort
    {
        public IPEndPoint endpoint;
        private UdpClient RecvClient;
        private UdpClient SendClient;
        private readonly int RecvPortNum;
        private readonly int SendPortNum;
        private readonly Action<string> msgfix;
        private readonly Action<string> changeVoltage;
        private readonly Regex volteageRegEx;
        public string ResposeString { get; set; } = "OK";
        public IPAddress LastPacketAddress { get; private set; } 

        public UdpSocketPort(int recv, int send , Action<string> msg, Action<string> changeVoltage)
        {
            this.RecvPortNum = recv;
            this.SendPortNum = send;
            this.msgfix = msg;
            this.changeVoltage = changeVoltage;
            this.volteageRegEx = new Regex(@"[\d ]+\.\d*V");
        }

        public void Comm_Start()
        {
            this.endpoint = new IPEndPoint(IPAddress.Any, this.RecvPortNum);
            this.RecvClient = new UdpClient(this.endpoint);
            this.RecvClient.BeginReceive(new AsyncCallback(this.ReceiveCallback), null);
            this.SendClient = new UdpClient(this.SendPortNum);
            this.msg("RecvClient.BeginReceive() R:{0} S:{1}", new object[] { this.RecvPortNum, this.SendPortNum });
        }

        public bool Output(string devName, int buffer)
        {
            byte[] senddata = Encoding.ASCII.GetBytes(string.Format("devName={0}, buffer={1}", devName, buffer));
            this.SendClient.Send(senddata, senddata.Length, "255.255.255.255", this.RecvPortNum);
            return true;
        }

        private void msg(string format , params object[] param)
        {
            msgfix(string.Format(format, param));
        }

        private void ReceiveCallback(IAsyncResult ar)
        {
            byte[] bytReceiveBuffer = new byte[255];
            bytReceiveBuffer = RecvClient.EndReceive(ar,ref endpoint);
            var strmsg = System.Text.Encoding.Default.GetString(bytReceiveBuffer);

            LastPacketAddress = endpoint.Address;
            changeVoltage(volteageRegEx.Match(strmsg).Value);
            
            strmsg = ResposeString + "\n";
            bytReceiveBuffer = System.Text.Encoding.Default.GetBytes(strmsg);
            RecvClient.BeginSend(bytReceiveBuffer, bytReceiveBuffer.Length, endpoint, this.receiveCompleted, RecvClient);
        }

        private void receiveCompleted(IAsyncResult ar)
        {
            RecvClient.BeginReceive(new AsyncCallback(this.ReceiveCallback), null);
        }

    }
}