#ifndef _connection_h_
#define _connection_h_

#include <v8.h>
#include <node.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <occi.h>
#include <oro.h>
#include "utils.h"
#include "nodeOracleException.h"
#include "executeBaton.h"

using namespace node;
using namespace v8;

/**
 * Wrapper for an OCCI Connection class so that it can be used in JavaScript
 */
class Connection: public ObjectWrap {
public:
  static Persistent<FunctionTemplate> s_ct;
  static void Init(Handle<Object> target);
  static NAN_METHOD(New);

  // asynchronous execute method
  static NAN_METHOD(Execute);
  static void EIO_Execute(uv_work_t* req);
  static void EIO_AfterExecute(uv_work_t* req, int status);

  // synchronous execute method
  static NAN_METHOD(ExecuteSync);

  // asynchronous commit method
  static NAN_METHOD(Commit);
  static void EIO_Commit(uv_work_t* req);
  static void EIO_AfterCommit(uv_work_t* req, int status);

  // asynchronous rollback method
  static NAN_METHOD(Rollback);
  static void EIO_Rollback(uv_work_t* req);
  static void EIO_AfterRollback(uv_work_t* req, int status);

  // asynchronous rollback method
  static NAN_METHOD(Close);
  static void EIO_Close(uv_work_t* req);
  static void EIO_AfterClose(uv_work_t* req, int status);

  static NAN_METHOD(Prepare);
  static NAN_METHOD(CreateReader);

  static NAN_METHOD(SetAutoCommit);
  static NAN_METHOD(SetPrefetchRowCount);
  static NAN_METHOD(IsConnected);

  static NAN_METHOD(CloseSync);

  void closeConnection();

  Connection();
  ~Connection();

  // Make Ref and Unref public so that ExecuteBaton can call them
  void Ref() {
    ObjectWrap::Ref();
  }
  void Unref() {
    ObjectWrap::Unref();
  }

  void setConnection(oracle::occi::Environment* environment,
      oracle::occi::StatelessConnectionPool* connectionPool,
      oracle::occi::Connection* connection);

  oracle::occi::Environment* getEnvironment() {
    return m_environment;
  }

  oracle::occi::StatelessConnectionPool* getConnectionPool() {
      return m_connectionPool;
  }

  oracle::occi::Connection* getConnection() {
    return m_connection;
  }

  bool getAutoCommit() {
    return m_autoCommit;
  }

  int getPrefetchRowCount() {
    return m_prefetchRowCount;
  }


  static int SetValuesOnStatement(oracle::occi::Statement* stmt,
      std::vector<value_t*> &values);
  static void CreateColumnsFromResultSet(oracle::occi::ResultSet* rs,
      std::vector<column_t*> &columns);
  static row_t* CreateRowFromCurrentResultSetRow(oracle::occi::ResultSet* rs,
      std::vector<column_t*> &columns);
  static Local<Array> CreateV8ArrayFromRows(std::vector<column_t*> columns,
      std::vector<row_t*>* rows);
  static Local<Object> CreateV8ObjectFromRow(std::vector<column_t*> columns,
      row_t* currentRow);

  // shared with Statement
  static oracle::occi::Statement* CreateStatement(ExecuteBaton* baton);
  static void ExecuteStatement(ExecuteBaton* baton, oracle::occi::Statement* stmt);

  static void handleResult(ExecuteBaton* baton, Handle<Value> (&argv)[2]);

private:
  // The OOCI environment
  oracle::occi::Environment* m_environment;

  // The underlying connection pool
  oracle::occi::StatelessConnectionPool* m_connectionPool;

  // The underlying connection
  oracle::occi::Connection* m_connection;

  // Autocommit flag
  bool m_autoCommit;

  // Prefetch row count
  int m_prefetchRowCount;

  static void EIO_AfterCall(uv_work_t* req, int status);
  static buffer_t* readClob(oracle::occi::Clob& clobVal);
  static buffer_t* readBlob(oracle::occi::Blob& blobVal);
};

/**
 * Wrapper for an OCCI StatelessConnectionPool class so that it can be used in JavaScript
 */
class ConnectionPool: public ObjectWrap {
public:
  static Persistent<FunctionTemplate> s_ct;

  static void Init(Handle<Object> target);
  static NAN_METHOD(New);
  static NAN_METHOD(GetInfo);
  static NAN_METHOD(CloseSync);

  static NAN_METHOD(GetConnection);
  static void EIO_GetConnection(uv_work_t* req);
  static void EIO_AfterGetConnection(uv_work_t* req, int status);

  static NAN_METHOD(Close);
  static void EIO_Close(uv_work_t* req);
  static void EIO_AfterClose(uv_work_t* req, int status);

  static NAN_METHOD(GetConnectionSync);

  void closeConnectionPool(
      oracle::occi::StatelessConnectionPool::DestroyMode mode =
          oracle::occi::StatelessConnectionPool::DEFAULT);

  ConnectionPool();
  ~ConnectionPool();

  void setConnectionPool(oracle::occi::Environment* environment,
      oracle::occi::StatelessConnectionPool* connectionPool);
  oracle::occi::Environment* getEnvironment() {
    return m_environment;
  }
  oracle::occi::StatelessConnectionPool* getConnectionPool() {
    return m_connectionPool;
  }

private:
  oracle::occi::Environment* m_environment;
  oracle::occi::StatelessConnectionPool* m_connectionPool;
};

/**
 * Baton for ConnectionPool
 */
class ConnectionPoolBaton {
public:
  ConnectionPoolBaton(oracle::occi::Environment* environment,
      ConnectionPool* connectionPool,
      oracle::occi::StatelessConnectionPool::DestroyMode destroyMode,
      const v8::Handle<v8::Function>& callback) {
    this->environment = environment;
    this->connectionPool = connectionPool;
    if (callback->IsUndefined()) {
      this->callback = NULL;
    } else {
      this->callback = new NanCallback(callback);
    }
    this->connection = NULL;
    this->error = NULL;
    this->destroyMode = destroyMode;
  }

  ~ConnectionPoolBaton() {
    delete error;
    delete callback;
  }

  oracle::occi::Environment* environment;
  ConnectionPool *connectionPool;
  oracle::occi::Connection* connection;
  NanCallback *callback;
  std::string *error;
  oracle::occi::StatelessConnectionPool::DestroyMode destroyMode;
};

class ConnectionBaton {
public:
  ConnectionBaton(Connection* connection, const v8::Handle<v8::Function>& callback) {
    this->connection = connection;
    if (callback.IsEmpty() || callback->IsUndefined()) {
      this->callback = NULL;
    } else {
      this->callback = new NanCallback(callback);
    }
    this->error = NULL;
  }
  ~ConnectionBaton() {
    delete error;
    delete callback;
  }

  Connection *connection;
  NanCallback *callback;
  std::string *error;
};

#endif
