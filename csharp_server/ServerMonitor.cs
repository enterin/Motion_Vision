using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace MFCServer1
{
    public static class ServerMonitor
    {
        // 상태 값들 (기존 그대로)
        public static bool TcpListening = false;
        public static int TcpPort = 9001;

        public static bool PythonAlive = false;
        public static string PythonLastError = "";

        public static string LastClient = "";
        public static DateTime LastClientTime = DateTime.MinValue;

        public static string LastResult = "";

        public static string LastTopImagePath = "";
        public static string LastSideImagePath = "";

        // 증가시키는 번호용
        private static int _nextId = 1;

        private static readonly List<InspectionRecord> _history =
            new List<InspectionRecord>();

        public class InspectionRecord
        {
            public int Id { get; set; }               // 번호
            public DateTime Time { get; set; }        // 시간
            public string Result { get; set; }        // "정상"/"비정상"
            public string Reason { get; set; }        // 불합격 사유
            public string TopPath { get; set; }       // TOP 이미지 경로
            public string SidePath { get; set; }      // SIDE 이미지 경로
        }

        public static IList<InspectionRecord> GetRecent()
        {
            return new List<InspectionRecord>(_history);
        }

        // result: "정상"/"비정상"
        // reason: "dent(0.91), scratch(0.7)" 이런 문자열
        // topPath / sidePath: 이미지 경로
        public static void AddLog(string result, string reason, string topPath, string sidePath)
        {
            _history.Add(new InspectionRecord
            {
                Id = _nextId++,
                Time = DateTime.Now,
                Result = result,
                Reason = reason,
                TopPath = topPath,
                SidePath = sidePath
            });

            if (_history.Count > 50)
            {
                _history.RemoveAt(0);
            }
        }

        public static async Task StartHealthCheck(PythonTcpClient py)
        {
            while (true)
            {
                try
                {
                    bool ok = await py.CheckHealthAsync();
                    PythonAlive = ok;
                }
                catch (Exception ex)
                {
                    PythonAlive = false;
                    PythonLastError = ex.Message;
                }

                await Task.Delay(1000);
            }
        }
    }
}
