using System;
using Android.App;
using Android.Widget;
using Android.OS;
using System.Net;
using System.Net.NetworkInformation;


namespace UdpList
{
    [Activity(Label = "UdpList", MainLauncher = true, Icon = "@drawable/icon")]
    public class MainActivity : Activity
    {

        private TextView txtHistory;
        private TextView txtVoltage;

        private UdpSocketPort UdpSocketPort;

        void msgfixed(string strmsg)
        {
            Application.SynchronizationContext.Post(
                _ =>
                {
                    txtHistory.Append(DateTime.Now.ToString("HH:mm:ss.fff"));
                    txtHistory.Append("> ");
                    txtHistory.Append(strmsg);
                    txtHistory.Append("\n");
                }
                , null);
        }

        void msg(string format, params object[] param )
        {
            Application.SynchronizationContext.Post(
                _ =>
                {

                    txtHistory.Append(DateTime.Now.ToString("HH:mm:ss.fff"));
                    txtHistory.Append("> ");
                    txtHistory.Append(string.Format(format, param));
                    txtHistory.Append("\n");
                }
                , null);
        }

        void changeVoltage(string msg)
        {
            Application.SynchronizationContext.Post(
                _ =>
                {
                    txtVoltage.Text = msg;
                }
                , null);
        }

        private int count = 0;

        protected override void OnCreate(Bundle bundle)
        {
            base.OnCreate(bundle);
            SetContentView(Resource.Layout.Main);
            Button button = FindViewById<Button>(Resource.Id.MyButton);

            button.Click += delegate {
                var ping = new Ping();
                var ipaddress = UdpSocketPort.LastPacketAddress ?? IPAddress.Parse("10.71.34.1");
                var func = new Action( delegate
                {
                    button.Text = string.Format("Pong!!! {0} {1}", ipaddress.ToString(), count);
                });
                
                ping.SendAsync(ipaddress, func );
                
                button.Text = string.Format("Ping... {0} {1}", ipaddress.ToString(), count++);
                msg(button.Text);
            };

            txtHistory = FindViewById<TextView>(Resource.Id.textView2);
            txtHistory.Text = "";

            txtVoltage = FindViewById<TextView>(Resource.Id.textView1);
            txtVoltage.Text = "";

            string hostname = Dns.GetHostName();
            msg("hostname ={0}", hostname);

            IPAddress[] adrList = Dns.GetHostAddresses(hostname);
            foreach (IPAddress address in adrList)
            {
                msg("address ={0}", address.ToString());
            }

            UdpSocketPort = new UdpSocketPort(11001, 11002, this.msgfixed, this.changeVoltage);
            UdpSocketPort.Comm_Start();
        }

    }
}

