# db_gencode

Gencode from sql script to cpp source files.

## 1. usage

```shell
./db_gencode <file_name>
```

---

## 2. gencode

```shell
make clean; make;
./db_gencode upload.sql
```

---

## 3. mysql scprit

upload.sql

```sql
DROP TABLE IF EXISTS `tb_file`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `tb_file` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `file_name` varchar(255) NOT NULL,
  `file_size` int(11) NOT NULL DEFAULT '0',
  `md5` char(32) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;
```

---

## 4. source file

tb_file.h \ tb_file.cpp

```c++
class tb_file {
public:
    tb_file();
    tb_file(const tb_file& obj);
    ~tb_file() {}

    tb_file& operator = (const tb_file& obj);

    inline const char* table_name() const { return "tb_file"; }
    inline tb_uint32 get_count() const { return 4; }
    string serialize() const;

    inline const char* col_id() const { return "`id`"; }
    inline const char* col_file_name() const { return "`file_name`"; }
    inline const char* col_file_size() const { return "`file_size`"; }
    inline const char* col_md5() const { return "`md5`"; }

    bool set_id(const tb_uint32 value);
    bool set_file_name(const string& value);
    bool set_file_name(const char* value, size_t size);
    bool set_file_name(const char* value);
    bool set_file_size(const tb_int32 value);
    bool set_md5(const string& value);
    bool set_md5(const char* value, size_t size);
    bool set_md5(const char* value);

    inline tb_uint32 id() const { return m_ui_id; }
    inline const string& file_name() const { return m_str_file_name; }
    inline tb_int32 file_size() const { return m_i_file_size; }
    inline const string& md5() const { return m_str_md5; }

    inline bool has_id() { return (m_ui_has_bit & 0x00000001) != 0; }
    inline bool has_file_name() { return (m_ui_has_bit & 0x00000002) != 0; }
    inline bool has_file_size() { return (m_ui_has_bit & 0x00000004) != 0; }
    inline bool has_md5() { return (m_ui_has_bit & 0x00000008) != 0; }

    inline void clear_has_id() { m_ui_has_bit &= ~0x00000001; }
    inline void clear_has_file_name() { m_ui_has_bit &= ~0x00000002; }
    inline void clear_has_file_size() { m_ui_has_bit &= ~0x00000004; }
    inline void clear_has_md5() { m_ui_has_bit &= ~0x00000008; }

    inline bool is_valid() const {return (m_ui_has_bit & 0x0000000f) != 0;}

private:
    tb_uint32 m_ui_has_bit;
    tb_uint32 m_ui_id;
    string m_str_file_name;
    tb_int32 m_i_file_size;
    string m_str_md5;
};
```

---

> 《[自动代码工具 - 分析 mysql 脚本（*.sql）生成 C++ 源码](https://wenfh2020.com/2020/06/04/mysql-db-gencode/)》
