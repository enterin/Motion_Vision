using System;
using MySql.Data.MySqlClient;

namespace MFCServer1
{
    public class DatabaseService
    {
        private readonly string _connStr;

        public DatabaseService(string connStr)
        {
            _connStr = connStr;
        }

        // 검수 결과 1건 INSERT
        // 테이블 가정:
        //    inspection_total(
        //        id INT AUTO_INCREMENT PRIMARY KEY,
        //        time DATETIME,
        //        result VARCHAR(50),
        //        image_paths TEXT
        //    )
        public void InsertInspection(string result, string imagePaths)
        {
            using (var conn = new MySqlConnection(_connStr))
            {
                conn.Open();

                string sql =
                    "INSERT INTO inspection_total(time, result, image_paths) " +
                    "VALUES (@time, @result, @image_paths)";

                using (var cmd = new MySqlCommand(sql, conn))
                {
                    cmd.Parameters.AddWithValue("@time", DateTime.Now);
                    cmd.Parameters.AddWithValue("@result", result);
                    cmd.Parameters.AddWithValue("@image_paths", imagePaths);
                    cmd.ExecuteNonQuery();
                }
            }
        }
    }
}
