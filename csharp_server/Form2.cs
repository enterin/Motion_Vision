using System;
using System.Linq;
using System.Windows.Forms;
using System.Drawing;
using System.IO;
using System.Collections.Generic;

namespace MFCServer1
{
    public partial class Form2 : Form
    {
        private Timer _timer;

        public Form2()
        {
            InitializeComponent();

            // =========================
            // DataGridView 스타일 / 기본 설정
            // =========================
            dataGridView1.AutoGenerateColumns = false;
            dataGridView1.Columns.Clear();

            dataGridView1.AllowUserToAddRows = false;
            dataGridView1.AllowUserToDeleteRows = false;
            dataGridView1.AllowUserToResizeRows = false;
            dataGridView1.MultiSelect = false;
            dataGridView1.SelectionMode = DataGridViewSelectionMode.FullRowSelect;
            dataGridView1.ReadOnly = true;
            dataGridView1.RowHeadersVisible = false;

            dataGridView1.RowTemplate.Height = 28;
            dataGridView1.DefaultCellStyle.Font = new Font("맑은 고딕", 9.5f, FontStyle.Regular);

            dataGridView1.EnableHeadersVisualStyles = false;
            dataGridView1.ColumnHeadersDefaultCellStyle.BackColor = Color.Gainsboro;
            dataGridView1.ColumnHeadersDefaultCellStyle.ForeColor = Color.Black;
            dataGridView1.ColumnHeadersDefaultCellStyle.Font =
                new Font("맑은 고딕", 10F, FontStyle.Bold);
            dataGridView1.ColumnHeadersDefaultCellStyle.Alignment =
                DataGridViewContentAlignment.MiddleCenter;
            dataGridView1.ColumnHeadersHeight = 30;

            dataGridView1.DefaultCellStyle.Alignment =
                DataGridViewContentAlignment.MiddleLeft;
            dataGridView1.DefaultCellStyle.Padding = new Padding(4, 2, 4, 2);

            dataGridView1.DefaultCellStyle.SelectionBackColor = Color.SteelBlue;
            dataGridView1.DefaultCellStyle.SelectionForeColor = Color.White;

            // =========================
            // 컬럼 구성 (번호 / 시간 / 결과 / 불합격 사유 / TOP 경로 / SIDE 경로)
            // =========================

            // 번호
            var colId = new DataGridViewTextBoxColumn
            {
                HeaderText = "번호",
                DataPropertyName = "Id",
                Width = 50,
                ReadOnly = true,
                DefaultCellStyle = new DataGridViewCellStyle
                {
                    Alignment = DataGridViewContentAlignment.MiddleCenter,
                    Font = new Font("맑은 고딕", 9.5f, FontStyle.Regular)
                }
            };

            // 시간
            var colTime = new DataGridViewTextBoxColumn
            {
                HeaderText = "시간",
                DataPropertyName = "Time",
                Width = 150,
                MinimumWidth = 150,
                ReadOnly = true,
                DefaultCellStyle = new DataGridViewCellStyle
                {
                    Alignment = DataGridViewContentAlignment.MiddleCenter,
                    Format = "yyyy-MM-dd HH:mm:ss",
                    Font = new Font("맑은 고딕", 9.5f, FontStyle.Regular)
                }
            };

            // 결과 (정상 / 비정상)
            var colResult = new DataGridViewTextBoxColumn
            {
                HeaderText = "결과",
                DataPropertyName = "Result",
                Width = 70,
                MinimumWidth = 70,
                ReadOnly = true,
                DefaultCellStyle = new DataGridViewCellStyle
                {
                    Alignment = DataGridViewContentAlignment.MiddleCenter,
                    Font = new Font("맑은 고딕", 10f, FontStyle.Bold),
                    ForeColor = Color.Black
                }
            };

            // 불합격 사유
            var colReason = new DataGridViewTextBoxColumn
            {
                HeaderText = "불합격 사유",
                DataPropertyName = "Reason",
                Width = 250,
                MinimumWidth = 200,
                ReadOnly = true,
                DefaultCellStyle = new DataGridViewCellStyle
                {
                    Alignment = DataGridViewContentAlignment.MiddleLeft,
                    Font = new Font("맑은 고딕", 9f, FontStyle.Regular)
                }
            };

            // TOP 경로
            var colTopPath = new DataGridViewTextBoxColumn
            {
                HeaderText = "TOP 경로",
                DataPropertyName = "TopPath",
                Width = 250,
                MinimumWidth = 200,
                ReadOnly = true,
                DefaultCellStyle = new DataGridViewCellStyle
                {
                    Alignment = DataGridViewContentAlignment.MiddleLeft,
                    Font = new Font("맑은 고딕", 9f, FontStyle.Regular)
                }
            };

            // SIDE 경로
            var colSidePath = new DataGridViewTextBoxColumn
            {
                HeaderText = "SIDE 경로",
                DataPropertyName = "SidePath",
                AutoSizeMode = DataGridViewAutoSizeColumnMode.Fill,
                ReadOnly = true,
                DefaultCellStyle = new DataGridViewCellStyle
                {
                    Alignment = DataGridViewContentAlignment.MiddleLeft,
                    Font = new Font("맑은 고딕", 9f, FontStyle.Regular)
                }
            };

            dataGridView1.Columns.Add(colId);
            dataGridView1.Columns.Add(colTime);
            dataGridView1.Columns.Add(colResult);
            dataGridView1.Columns.Add(colReason);
            dataGridView1.Columns.Add(colTopPath);
            dataGridView1.Columns.Add(colSidePath);

            // 결과 색상 강조 (정상=초록 / 비정상=빨강)
            dataGridView1.CellFormatting += (s, e) =>
            {
                if (dataGridView1.Columns[e.ColumnIndex].DataPropertyName == "Result" &&
                    e.Value != null)
                {
                    string val = e.Value.ToString();
                    if (val.Contains("비정상"))
                    {
                        e.CellStyle.ForeColor = Color.Red;
                    }
                    else if (val.Contains("정상"))
                    {
                        e.CellStyle.ForeColor = Color.Green;
                    }
                }
            };

            // =========================
            // 타이머 설정
            // =========================
            _timer = new Timer();
            _timer.Interval = 1000; // 1초마다 상태/로그 갱신
            _timer.Tick += Timer_Tick;
            _timer.Start();
        }

        // ---------------------------------
        // 주기적으로 UI 갱신
        // ---------------------------------
        private void Timer_Tick(object sender, EventArgs e)
        {
            // ---- TCP 상태 표시 ----
            if (ServerMonitor.TcpListening)
            {
                lblTcpStatus.Text = $"TCP LISTENING : Port {ServerMonitor.TcpPort}";
                lblTcpStatus.ForeColor = Color.LimeGreen;
            }
            else
            {
                lblTcpStatus.Text = "TCP STOPPED";
                lblTcpStatus.ForeColor = Color.Red;
            }

            // ---- Python 상태 표시 ----
            if (ServerMonitor.PythonAlive)
            {
                lblPythonStatus.Text = "PYTHON OK";
                lblPythonStatus.ForeColor = Color.LimeGreen;
            }
            else
            {
                lblPythonStatus.Text = "PYTHON DOWN";
                lblPythonStatus.ForeColor = Color.Red;
            }

            // ---- 기타 상태 라벨 ----
            lblPythonError.Text = "LastError: " + ServerMonitor.PythonLastError;
            lblLastClient.Text =
                $"LastClient: {ServerMonitor.LastClient} at {ServerMonitor.LastClientTime:HH:mm:ss}";
            lblLastResult.Text = $"LastResult: {ServerMonitor.LastResult}";

            // ---- 로그 테이블 갱신 ----
            // ServerMonitor에서 최근 로그 히스토리를 가져와서
            // 최신순(최근이 위로 오도록) 정렬 후 바인딩
            var latestLogs = ServerMonitor
                .GetRecent()
                .OrderByDescending(x => x.Time)
                .ToList();

            dataGridView1.DataSource = latestLogs;
            dataGridView1.Refresh();

            // ---- 마지막 검사 이미지 자동 미리보기 ----
            ShowImages(ServerMonitor.LastTopImagePath, ServerMonitor.LastSideImagePath);
        }

        // ---------------------------------
        // 로그 행 클릭 시 해당 이미지 표시
        // ---------------------------------
        private void dataGridView1_CellClick(object sender, DataGridViewCellEventArgs e)
        {
            if (e.RowIndex < 0) return; // 헤더 클릭이면 무시

            var rowObj = dataGridView1.Rows[e.RowIndex].DataBoundItem
                         as ServerMonitor.InspectionRecord;
            if (rowObj == null) return;

            ShowImages(rowObj.TopPath, rowObj.SidePath);
        }

        // ---------------------------------
        // PictureBox에 이미지 올리기
        // ---------------------------------
        private void ShowImages(string topPath, string sidePath)
        {
            // TOP
            if (!string.IsNullOrEmpty(topPath) && File.Exists(topPath))
            {
                try
                {
                    picTop.Image = LoadImageNoLock(topPath);
                }
                catch
                {
                    // 이미지 로드 실패해도 UI 안 죽게 무시
                }
            }

            // SIDE
            if (!string.IsNullOrEmpty(sidePath) && File.Exists(sidePath))
            {
                try
                {
                    picSide.Image = LoadImageNoLock(sidePath);
                }
                catch
                {
                    // 무시
                }
            }
        }

        // 파일을 바로 Image.FromFile로 열면 파일 잠금이 걸릴 수 있으므로
        // MemoryStream 복사 방식으로 안전하게 로드
        private Image LoadImageNoLock(string path)
        {
            using (var fs = new FileStream(
                path,
                FileMode.Open,
                FileAccess.Read,
                FileShare.ReadWrite))
            {
                var ms = new MemoryStream();
                fs.CopyTo(ms);
                ms.Position = 0;
                return Image.FromStream(ms);
            }
        }
    }
}
