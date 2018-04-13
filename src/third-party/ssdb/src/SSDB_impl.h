#ifndef SSDB_API_IMPL_CPP
#define SSDB_API_IMPL_CPP

#include "SSDB.h"
#include "net/link.h"

namespace ssdb {

class ClientImpl : public Client {
 private:
  friend class Client;

  Link* link;
  std::vector<std::string> resp_;

 public:
  ClientImpl();
  ~ClientImpl();

  virtual const std::vector<std::string>* request(const std::vector<std::string>& req) override;
  virtual const std::vector<std::string>* request(const std::string& cmd) override;
  virtual const std::vector<std::string>* request(const std::string& cmd, const std::string& s2) override;
  virtual const std::vector<std::string>* request(const std::string& cmd,
                                                  const std::string& s2,
                                                  const std::string& s3) override;
  virtual const std::vector<std::string>* request(const std::string& cmd,
                                                  const std::string& s2,
                                                  const std::string& s3,
                                                  const std::string& s4) override;
  virtual const std::vector<std::string>* request(const std::string& cmd,
                                                  const std::string& s2,
                                                  const std::string& s3,
                                                  const std::string& s4,
                                                  const std::string& s5) override;
  virtual const std::vector<std::string>* request(const std::string& cmd,
                                                  const std::string& s2,
                                                  const std::string& s3,
                                                  const std::string& s4,
                                                  const std::string& s5,
                                                  const std::string& s6) override;
  virtual const std::vector<std::string>* request(const std::string& cmd, const std::vector<std::string>& s2) override;
  virtual const std::vector<std::string>* request(const std::string& cmd,
                                                  const std::string& s2,
                                                  const std::vector<std::string>& s3) override;

#ifdef FASTO
  virtual Status auth(const std::string& password) override;
  virtual Status expire(const std::string& key, int ttl) override;
  virtual Status ttl(const std::string& key, int* ttl) override;
#endif
  virtual Status dbsize(int64_t* ret) override;
  virtual Status get_kv_range(std::string* start, std::string* end) override;
  virtual Status set_kv_range(const std::string& start, const std::string& end) override;

  virtual Status get(const std::string& key, std::string* val) override;
  virtual Status set(const std::string& key, const std::string& val) override;
  virtual Status setx(const std::string& key, const std::string& val, int ttl) override;
  virtual Status del(const std::string& key) override;
  virtual Status incr(const std::string& key, int64_t incrby, int64_t* ret) override;
  virtual Status keys(const std::string& key_start,
                      const std::string& key_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) override;
  virtual Status scan(const std::string& key_start,
                      const std::string& key_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) override;
  virtual Status rscan(const std::string& key_start,
                       const std::string& key_end,
                       uint64_t limit,
                       std::vector<std::string>* ret) override;
  virtual Status multi_get(const std::vector<std::string>& keys, std::vector<std::string>* ret) override;
  virtual Status multi_set(const std::map<std::string, std::string>& kvs) override;
  virtual Status multi_del(const std::vector<std::string>& keys) override;

  virtual Status hget(const std::string& name, const std::string& key, std::string* val) override;
  virtual Status hset(const std::string& name, const std::string& key, const std::string& val) override;
  virtual Status hdel(const std::string& name, const std::string& key) override;
  virtual Status hincr(const std::string& name, const std::string& key, int64_t incrby, int64_t* ret) override;
  virtual Status hsize(const std::string& name, int64_t* ret) override;
  virtual Status hclear(const std::string& name, int64_t* ret = NULL) override;
  virtual Status hkeys(const std::string& name,
                       const std::string& key_start,
                       const std::string& key_end,
                       uint64_t limit,
                       std::vector<std::string>* ret) override;
  virtual Status hgetall(const std::string& name, std::vector<std::string>* ret) override;
  virtual Status hscan(const std::string& name,
                       const std::string& key_start,
                       const std::string& key_end,
                       uint64_t limit,
                       std::vector<std::string>* ret) override;
  virtual Status hrscan(const std::string& name,
                        const std::string& key_start,
                        const std::string& key_end,
                        uint64_t limit,
                        std::vector<std::string>* ret) override;
  virtual Status multi_hget(const std::string& name,
                            const std::vector<std::string>& keys,
                            std::vector<std::string>* ret) override;
  virtual Status multi_hset(const std::string& name, const std::map<std::string, std::string>& kvs) override;
  virtual Status multi_hdel(const std::string& name, const std::vector<std::string>& keys) override;

  virtual Status zget(const std::string& name, const std::string& key, int64_t* ret) override;
  virtual Status zset(const std::string& name, const std::string& key, int64_t score) override;
  virtual Status zdel(const std::string& name, const std::string& key) override;
  virtual Status zincr(const std::string& name, const std::string& key, int64_t incrby, int64_t* ret) override;
  virtual Status zsize(const std::string& name, int64_t* ret) override;
  virtual Status zclear(const std::string& name, int64_t* ret = NULL) override;
  virtual Status zrank(const std::string& name, const std::string& key, int64_t* ret) override;
  virtual Status zrrank(const std::string& name, const std::string& key, int64_t* ret) override;
  virtual Status zrange(const std::string& name,
                        uint64_t offset,
                        uint64_t limit,
                        std::vector<std::string>* ret) override;
  virtual Status zrrange(const std::string& name,
                         uint64_t offset,
                         uint64_t limit,
                         std::vector<std::string>* ret) override;
  virtual Status zkeys(const std::string& name,
                       const std::string& key_start,
                       int64_t* score_start,
                       int64_t* score_end,
                       uint64_t limit,
                       std::vector<std::string>* ret) override;
  virtual Status zscan(const std::string& name,
                       const std::string& key_start,
                       int64_t* score_start,
                       int64_t* score_end,
                       uint64_t limit,
                       std::vector<std::string>* ret) override;
  virtual Status zrscan(const std::string& name,
                        const std::string& key_start,
                        int64_t* score_start,
                        int64_t* score_end,
                        uint64_t limit,
                        std::vector<std::string>* ret) override;
  virtual Status multi_zget(const std::string& name,
                            const std::vector<std::string>& keys,
                            std::vector<std::string>* scores) override;
  virtual Status multi_zset(const std::string& name, const std::map<std::string, int64_t>& kss) override;
  virtual Status multi_zdel(const std::string& name, const std::vector<std::string>& keys) override;

  virtual Status qpush(const std::string& name, const std::string& item, int64_t* ret_size = NULL) override;
  virtual Status qpush(const std::string& name,
                       const std::vector<std::string>& items,
                       int64_t* ret_size = NULL) override;
  virtual Status qpop(const std::string& name, std::string* item) override;
  virtual Status qpop(const std::string& name, int64_t limit, std::vector<std::string>* ret) override;
  virtual Status qslice(const std::string& name, int64_t begin, int64_t end, std::vector<std::string>* ret) override;
  virtual Status qrange(const std::string& name, int64_t begin, int64_t limit, std::vector<std::string>* ret) override;
  virtual Status qclear(const std::string& name, int64_t* ret = NULL) override;
#ifdef FASTO
  virtual Status info(const std::string& args, std::vector<std::string>* ret) override;
#endif
};

}  // namespace ssdb

#endif
