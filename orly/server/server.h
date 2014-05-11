/* <orly/server/server.h>

   The Orly server.

   Copyright 2010-2014 OrlyAtomics, Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#pragma once

#include <cassert>
#include <memory>
#include <mutex>
#include <unordered_map>

#include <base/class_traits.h>
#include <base/debug_log.h>
#include <base/fd.h>
#include <base/log.h>
#include <base/scheduler.h>
#include <base/timer_fd.h>
#include <base/uuid.h>
#include <socket/address.h>
#include <orly/durable/kit.h>
#include <orly/indy/manager.h>
#include <orly/indy/disk/durable_manager.h>
#include <orly/indy/disk/indy_util_reporter.h>
#include <orly/indy/disk/sim/mem_engine.h>
#include <orly/indy/disk/util/disk_engine.h>
#include <orly/indy/fiber/fiber.h>
#include <orly/indy/fiber/jump_runnable.h>
#include <orly/notification/all.h>
#include <orly/notification/pov_failure.h>
#include <orly/notification/system_shutdown.h>
#include <orly/notification/update_progress.h>
#include <orly/package/manager.h>
#include <orly/server/repo_tetris_manager.h>
#include <orly/server/session.h>
#include <orly/server/ws.h>
#include <orly/type/type_czar.h>

namespace Orly {

  namespace Server {

    /* TODO */
    class TIndexType {
      public:

      /* TODO */
      TIndexType(Indy::TKey &&key, Indy::TKey &&val) : Key(std::move(key)), Val(std::move(val)) {}

      /* TODO */
      inline size_t GetHash() const {
        assert(this);
        return Key.GetHash() ^ Val.GetHash();
      }

      /* TODO */
      inline bool operator==(const TIndexType &that) const {
        assert(this);
        return Key == that.Key && Val == that.Val;
      }

      /* TODO */
      inline const Indy::TKey &GetKey() const {
        assert(this);
        return Key;
      }

      /* TODO */
      inline const Indy::TKey &GetVal() const {
        assert(this);
        return Val;
      }

      private:

      /* TODO */
      Indy::TKey Key;
      Indy::TKey Val;

    };  // TIndexType

  }  // Server

}  // Orly

namespace std {

  /* A standard hasher for Orly::Server::TIndexType. */
  template <>
  struct hash<Orly::Server::TIndexType> {
    typedef size_t result_type;
    typedef Orly::Server::TIndexType argument_type;
    size_t operator()(const argument_type &that) const {
      return that.GetHash();
    }
  };

}  // std

namespace Orly {

  namespace Server {

    /* Forward Declarations. */
    class TServer;

    /* TODO */
    class TIndyReporter {
      NO_COPY(TIndyReporter);
      public:

      /* TODO */
      TIndyReporter(const TServer *server, Base::TScheduler *scheduler, int port_number);

      private:

        /* TODO */
      void AcceptClientConnections();

      /* TODO */
      void ServeClient(Base::TFd &fd);

      /* TODO */
      void AddReport(std::stringstream &ss) const;

      /* TODO */
      const TServer *Server;

      /* TODO */
      Base::TFd Socket;

      /* TODO */
      Base::TScheduler *Scheduler;

      /* TODO */
      mutable Base::TTimer ReportTimer;

      /* TODO */
      typedef std::chrono::system_clock TClock;

      /* TODO */
      typedef std::chrono::time_point<TClock, std::chrono::nanoseconds> TTimePoint;

      static const size_t NanoToSecond = 1000000000UL;

    };  // TIndyReporter

    /* TODO */
    class TServer final
        : public TSession::TServer, public Indy::Fiber::TRunnable,
          public TWs::TSessionManager {
      NO_COPY(TServer);
      public:

      /* TODO */
      static constexpr size_t NumSlowRunners = 8UL;

      /* TODO */
      using TTimePoint = std::chrono::time_point<std::chrono::system_clock>;

      /* Command-line arguments. */
      class TCmd
          : public Base::TLog::TCmd {
        public:

        /* The port on which TServer::MainSocket listens for clients. */
        in_port_t PortNumber;

        /* The port on which the server listens for websocket clients. */
        in_port_t WsPortNumber;

        /* Enables the memcache interface */
        bool EnableMemcache;

        /* The port on which TServer::MemcacheSocket listens for clients. */
        in_port_t MemcachePortNumber;

        /* The port on which TServer::WaitForSlave listens for a slave. */
        in_port_t SlavePortNumber;

        /* The maximum number of connection requests to backlog against MainSocket. */
        int ConnectionBacklog;

        /* The maximum number of durable objects to keep cached in memory. */
        size_t DurableCacheSize;

        /* The maximum number of milliseconds a connection can remain idle before the server hangs up. */
        size_t IdleConnectionTimeout;

        /* The minimum number of milliseconds between rounds of housecleaning. */
        size_t HousecleaningInterval;

        /* The minimum number of milliseconds between rounds of layer cleaning. */
        size_t LayerCleaningInterval;

        /* Run in memory simulation mode. */
        bool MemorySim;

        /* The amount of memory in MB to allocate for a memory simulated volume. */
        size_t MemorySimMB;

        /* The amount of memory in MB to allocate for a memory simulated slow volume. */
        size_t MemorySimSlowMB;

        /* The number of files that can be in a single generation of temporary files before they get merged into the next generation. */
        size_t TempFileConsolidationThreshold;

        /* TODO */
        std::string InstanceName;

        /* TODO */
        size_t PageCacheSizeMB;

        /* TODO */
        size_t BlockCacheSizeMB;

        /* TODO */
        size_t FileServiceAppendLogMB;

        /* TODO */
        size_t DiskMaxAioNum;

        /* TODO */
        double HighDiskUtilizationThreshold;

        /* TODO */
        bool DiscardOnCreate;

        /* TODO */
        size_t ReplicationSyncBufMB;

        /* TODO */
        size_t MergeMemInterval;

        /* TODO */
        size_t MergeDiskInterval;

        /* TODO */
        size_t ReplicationInterval;

        /* TODO */
        size_t DurableWriteInterval;

        /* TODO */
        size_t DurableMergeInterval;

        /* TODO */
        std::string StartingState;

        /* TODO */
        size_t NumMemMergeThreads;

        /* TODO */
        size_t NumDiskMergeThreads;

        /* TODO */
        size_t MaxRepoCacheSize;

        /* TODO */
        std::vector<size_t> FastCoreVec;

        /* TODO */
        std::vector<size_t> SlowCoreVec;

        /* TODO */
        std::vector<size_t> DiskControllerCoreVec;

        /* TODO */
        std::vector<size_t> MemMergeCoreVec;

        /* TODO */
        std::vector<size_t> DiskMergeCoreVec;

        /* TODO */
        size_t NumFiberFrames;

        /* TODO */
        size_t NumDiskEvents;

        /* The port on which we respond to HTTP by giving a status report. */
        in_port_t ReportingPortNumber;

        /* Support for tailing. */
        bool AllowTailing;

        /* We can use this to sync when the data layout has changed. */
        bool AllowFileSync;

        /* If true, realtime thread priorities will not be used (realtime
           priorities require root privileges). */
        bool NoRealtime;

        /* Controls whether fsync is used when writing to disk */
        bool DoFsync;

        /* TODO */
        bool LogAssertionFailures;

        /******** Object Pools ********/

        /* TODO */
        size_t DurableMappingPoolSize;
        size_t DurableMappingEntryPoolSize;
        size_t DurableLayerPoolSize;
        size_t DurableMemEntryPoolSize;

        /* TODO */
        size_t RepoMappingPoolSize;
        size_t RepoMappingEntryPoolSize;
        size_t RepoDataLayerPoolSize;

        /* TODO */
        size_t TransactionMutationPoolSize;
        size_t TransactionPoolSize;

        /* TODO */
        size_t UpdatePoolSize;
        size_t UpdateEntryPoolSize;

        /* TODO */
        size_t DiskBufferBlockPoolSize;

        /******** End Object Pools ********/

        /* TODO */
        Socket::TAddress AddressOfMaster;

        /* TODO */
        std::string PackageDirectory;

        /* TODO */
        bool Create;

        protected:

        /* Our meta-type. */
        class TMeta
            : public Base::TLog::TCmd::TMeta {
          protected:

          /* Registers our fields. */
          TMeta(const char *desc);

        };  // TServer::TCmd::TMeta

        /* Construct with defaults. */
        TCmd();

      };  // TServer::TCmd

      /* Launches background tasks using the given scheduler and takes its arguments from the cmd object. */
      TServer(Base::TScheduler *scheduler, const TCmd &cmd);

      /* Handles all the complicated aspects of TServer's constructor in a separate frame. */
      void Init();

      /* Clean up and go. */
      virtual ~TServer();

      /* See TSession::TServer. */
      virtual const std::shared_ptr<Durable::TManager> &GetDurableManager() const override {
        assert(this);
        return DurableManager;
      }

      /* See TPov::TServer. */
      virtual const Indy::L0::TManager::TPtr<Indy::TRepo> &GetGlobalRepo() const override {
        assert(this);
        return GlobalRepo;
      }

      /* See TSession::TServer. */
      virtual const Package::TManager &GetPackageManager() const override {
        assert(this);
        return PackageManager;
      }

      /* See TPov::TServer. */
      virtual Orly::Indy::TManager *GetRepoManager() const override {
        assert(this);
        return RepoManager.get();
      }

      /* TODO */
      virtual Base::TScheduler *GetScheduler() const override {
        assert(this);
        assert(Scheduler);
        return Scheduler;
      }

      /* Called when the websockets server wishes to create a new session. */
      virtual TWs::TSessionPin *NewSession() override;

      /* Called when the websockets server wishes to resume an old session. */
      virtual TWs::TSessionPin *ResumeSession(const Base::TUuid &id) override;

      private:

      /* A live connection to a client. */
      class TConnection final
          : public Rpc::TContext {
        NO_COPY(TConnection);
        public:

        /* The session associated with this connection.  Never null. */
        const Durable::TPtr<TSession> &GetSession() const {
          assert(this);
          return Session;
        }

        /* Ignore me.  I'm just here temporarily. */
        std::string Echo(const std::string &msg) {
          return Server->Echo(msg);
        }

        /* See <orly/protocol.h>. */
        void InstallPackage(const std::vector<std::string> &package_name, uint64_t version) {
          assert(this);
          Server->InstallPackage(package_name, version);
        }

        /* See <orly/protocol.h>. */
        Base::TUuid NewFastPrivatePov(const Base::TOpt<Base::TUuid> &parent_pov_id, const std::chrono::seconds &time_to_live) {
          assert(this);
          return Session->NewFastPrivatePov(Server, parent_pov_id, time_to_live);
        }

        /* See <orly/protocol.h>. */
        Base::TUuid NewSafePrivatePov(const Base::TOpt<Base::TUuid> &parent_pov_id, const std::chrono::seconds &time_to_live) {
          assert(this);
          return Session->NewSafePrivatePov(Server, parent_pov_id, time_to_live);
        }

        /* See <orly/protocol.h>. */
        Base::TUuid NewFastSharedPov(const Base::TOpt<Base::TUuid> &parent_pov_id, const std::chrono::seconds &time_to_live) {
          assert(this);
          return Session->NewFastSharedPov(Server, parent_pov_id, time_to_live);
        }

        /* See <orly/protocol.h>. */
        Base::TUuid NewSafeSharedPov(const Base::TOpt<Base::TUuid> &parent_pov_id, const std::chrono::seconds &time_to_live) {
          assert(this);
          return Session->NewSafeSharedPov(Server, parent_pov_id, time_to_live);
        }

        /* See <orly/protocol.h>. */
        void SetTimeToLive(const Base::TUuid &durable_id, const std::chrono::seconds &time_to_live) {
          assert(this);
          Session->SetTimeToLive(Server, durable_id, time_to_live);
        }

        /* See <orly/protocol.h>. */
        void SetUserId(const Base::TUuid &user_id) {
          assert(this);
          Session->SetUserId(Server, user_id);
        }

        /* See <orly/protocol.h>. */
        void PausePov(const Base::TUuid &pov_id) {
          assert(this);
          Session->PausePov(Server, pov_id);
        }

        /* See <orly/protocol.h>. */
        void UnpausePov(const Base::TUuid &pov_id) {
          assert(this);
          Session->UnpausePov(Server, pov_id);
        }

        /* See <orly/protocol.h>. */
        void UninstallPackage(const std::vector<std::string> &package_name, uint64_t version) {
          assert(this);
          Server->UninstallPackage(package_name, version);
        }

        /* See <orly/protocol.h>. */
        TMethodResult Try(const Base::TUuid &pov_id, const std::vector<std::string> &fq_name, const TClosure &closure) {
          assert(this);
          return Session->Try(Server, pov_id, fq_name, closure);
        }

        /* See <orly/protocol.h>. */
        TMethodResult TryTracked(const Base::TUuid &pov_id, const std::vector<std::string> &fq_name, const TClosure &closure) {
          assert(this);
          return Session->TryTracked(Server, pov_id, fq_name, closure);
        }

        /* See <orly/protocol.h>. */
        TMethodResult DoInPast(
            const Base::TUuid &pov_id, const std::vector<std::string> &fq_name, const TClosure &closure, const Base::TUuid &tracking_id) {
          return Session->DoInPast(Server, pov_id, fq_name, closure, tracking_id);
        }

        /* See <orly/protocol.h>. */
        void BeginImport() {
          assert(this);
          Server->BeginImport();
        }

        /* See <orly/protocol.h>. */
        void EndImport() {
          assert(this);
          Server->EndImport();
        }

        /* See <orly/protocol.h>. */
        void TailGlobalPov() {
          assert(this);
          Server->TailGlobalPov();
        }

        /* See <orly/protocol.h>. */
        std::string ImportCoreVector(const std::string &file_pattern, int64_t num_load_threads, int64_t num_merge_threads, int64_t merge_simultaneous) {
          assert(this);
          return Server->ImportCoreVector(file_pattern, num_load_threads, num_merge_threads, merge_simultaneous);
        }

        /* Run the RPC I/O with the client.  This function is called by ServeClient() after the handshake has
           been negotiated.  This function doesn't return until the client hangs up.  It will throw exceptions
           if the client times out, commits a syntax error, or otherwise does something weird. */
        void Run(Base::TFd &fd);

        /* Run the given jump-runnable on the server's websockets runner. */
        void RunWs(Indy::Fiber::TJumpRunnable &&jump_runnable) {
          assert(this);
          Server->RunWs(std::move(jump_runnable));
        }

        /* Construct a new connection for the given server, connected to the given session.  Neither the server
           pointer nor the session pointer may be null.  If the server already has a connection to the given session,
           this function returns null. */
        static std::shared_ptr<TConnection> New(TServer *server, const Durable::TPtr<TSession> &session);

        class TConnectionRunnable
            : public Indy::Fiber::TRunnable {
          NO_COPY(TConnectionRunnable);
          public:

          TConnectionRunnable(Indy::Fiber::TRunner *runner, const std::shared_ptr<const Rpc::TAnyRequest> &request);

          ~TConnectionRunnable();

          void Compute();

          private:

          Base::TThreadLocalGlobalPoolManager<Indy::Fiber::TFrame, size_t, Indy::Fiber::TRunner *>::TThreadLocalPool *FramePool;

          Indy::Fiber::TFrame *Frame;

          std::shared_ptr<const Rpc::TAnyRequest> Request;

        };


        private:

        /* The RPC protocol spoken by our end of this conversation. */
        class TProtocol
            : public Rpc::TProtocol {
          NO_COPY(TProtocol);
          public:

          /* Our singleton. */
          static const TProtocol Protocol;

          private:

          /* Registers the RPC entry points. */
          TProtocol();

        };  // TServer::TConnection::TProtocol

        /* TODO */
        TConnection(TServer *server, const Durable::TPtr<TSession> &session);

        /* TODO */
        static void OnRelease(TConnection *connection);

        /* TODO */
        TServer *const Server;

        /* TODO */
        const Durable::TPtr<TSession> Session;

      };  // TServer::TConnection

      /* Constructed by NewSession() and ResumeSession() to hold a session open for
         the websockets server. */
      class TSessionPin final
          : public TWs::TSessionPin {
        public:

        /* Construct a new session. */
        explicit TSessionPin(TServer *server);

        /* Resume an old session. */
        TSessionPin(TServer *server, const Base::TUuid &id);

        /* Pass-throughs to the session object we have pinned. */
        virtual void BeginImport() const override;
        virtual void EndImport() const override;
        virtual const Base::TUuid &GetId() const override;
        virtual void Import(const std::string &, int64_t, int64_t, int64_t) const override;
        virtual void InstallPackage(const std::vector<std::string> &, uint64_t) const override;
        virtual Base::TUuid NewPov(bool, bool, const Base::TOpt<Base::TUuid> &) const override;
        virtual void PausePov(const Base::TUuid &) const override;
        virtual void SetTtl(const Base::TUuid &, const std::chrono::seconds &) const override;
        virtual void SetUserId(const Base::TUuid &) const override;
        virtual void Tail() const override;
        virtual TMethodResult Try(const TMethodRequest &) const override;
        virtual void UninstallPackage(const std::vector<std::string> &, uint64_t) const override;
        virtual void UnpausePov(const Base::TUuid &) const override;

        private:

        /* Our link to the reset of the server. */
        std::shared_ptr<TConnection> Conn;

      };  // TServer::TSessionPin

      /* TODO */
      class TServeClientRunnable
          : public Indy::Fiber::TRunnable {
        NO_COPY(TServeClientRunnable);
        public:

        /* TODO */
        TServeClientRunnable(TServer *server, Indy::Fiber::TRunner *runner, Base::TFd &&fd, const Socket::TAddress &client_address, bool is_memcache)
            : Server(server),
              Fd(fd),
              ClientAddress(client_address),
              IsMemcache(is_memcache) {
          FramePool = Indy::Fiber::TFrame::LocalFramePool;
          Frame = FramePool->Alloc();
          try {
              Frame->Latch(runner, this, static_cast<Indy::Fiber::TRunnable::TFunc>(&TServeClientRunnable::Serve));
          } catch (...) {
            FramePool->Free(Frame);
            throw;
          }
        }

        /* TODO */
        virtual ~TServeClientRunnable() {
          assert(this);
        }

        /* TODO */
        void Serve() {
          assert(this);
          if(IsMemcache) {
            Server->ServeMemcacheClient(std::move(Fd), ClientAddress);
          } else {
            Server->ServeClient(Fd, ClientAddress);
          }
          Indy::Fiber::FreeMyFrame(FramePool);
          delete this;
        }

        private:

        /* TODO */
        TServer *Server;

        /* TODO */
        Base::TThreadLocalGlobalPoolManager<Indy::Fiber::TFrame, size_t, Indy::Fiber::TRunner *>::TThreadLocalPool *FramePool;
        Indy::Fiber::TFrame *Frame;

        /* TODO: Push <strm/fd.h> up to here, that should own our Fd and be our common Io library */
        Base::TFd Fd;

        /* TODO */
        const Socket::TAddress ClientAddress;

        bool IsMemcache;

      };  // TServeClientRunnable

      /* Accepts connections from clients on our main socket.  Launched as a thread by the constructor. */
      void AcceptClientConnections(bool is_memcache);

      /* See <orly/protocol.h>. */
      void BeginImport();

      /* Performs housecleaning operations at a rate regulated by HousecleaningTimer. */
      void CleanHouse();

      /* Ignore me! */
      std::string Echo(const std::string &msg);

      /* See <orly/protocol.h>. */
      void EndImport();

      /* See <orly/protocol.h>. */
      void TailGlobalPov();

      /* See <orly/protocol.h>. */
      std::string Import(const std::string &file, int64_t xact_count);

      /* See <orly/protocol.h>. */
      std::string ImportCoreVector(const std::string &file_pattern, int64_t num_load_threads, int64_t num_merge_threads, int64_t merge_simultaneous);

      /* See <orly/protocol.h>. */
      void InstallPackage(const std::vector<std::string> &package_name, uint64_t version);

      /* Run the given jump-runnable on our websockets runner. */
      void RunWs(Indy::Fiber::TJumpRunnable &&jump_runnable) {
        assert(this);
        jump_runnable(FramePoolManager.get(), &WsRunner);
      }

      /* Serves a client on the given fd.  Launched as a thread by AcceptClientConnections() when a client connects. */
      void ServeClient(Base::TFd &fd, const Socket::TAddress &client_address);

      /* Serves a client which speaks the memcached protocol. */
      void ServeMemcacheClient(Base::TFd &&fd, const Socket::TAddress &client_address);

      /* TODO */
      void StateChangeCb(Orly::Indy::TManager::TState state);

      /* See <orly/protocol.h>. */
      void UninstallPackage(const std::vector<std::string> &package_name, uint64_t version);

      /* TODO */
      void WaitForSlave();

      /* TODO */
      Indy::Fiber::TFrame *Frame;

      /* TODO */
      std::vector<Indy::Fiber::TFrame *> MergeMemFrameVec;
      std::vector<std::unique_ptr<Indy::Fiber::TRunner>> MergeMemRunnerVec;
      std::vector<Indy::Fiber::TFrame *> MergeDiskFrameVec;
      std::vector<std::unique_ptr<Indy::Fiber::TRunner>> MergeDiskRunnerVec;
      Indy::Fiber::TRunner DurableLayerCleanerRunner;
      Indy::Fiber::TRunner RepoLayerCleanerRunner;
      Indy::Fiber::TRunner BGFastRunner;
      Indy::Fiber::TRunner WaitForSlaveRunner;
      Indy::Fiber::TRunner RunReplicationQueueRunner;
      Indy::Fiber::TRunner RunReplicationWorkRunner;
      Indy::Fiber::TRunner RunReplicateTransactionRunner;
      Indy::Fiber::TRunner WsRunner;
      std::unordered_set<Indy::Fiber::TRunner *> ForEachSchedCallbackExtraSet;

      /* TODO */
      std::unordered_map<TIndexType, Base::TUuid> IndexTypeByIdMap;
      std::unordered_set<Base::TUuid> IndexIdSet;
      Atom::TSuprena IndexMapArena;
      std::mutex IndexMapMutex;

      /* Used for simulation, backed by memory. */
      std::unique_ptr<Indy::Disk::Sim::TMemEngine> SimMemEngine;

      /* Used when not running in memory simulation mode. */
      std::unique_ptr<Indy::Disk::Util::TDiskEngine> DiskEngine;

      /* TODO */
      std::unique_ptr<Orly::Indy::TManager> RepoManager;

      /* TODO */
      Orly::Indy::L0::TManager::TPtr<Indy::TRepo> GlobalRepo;

      /* TODO */
      Orly::Indy::TManager::TState RepoState;

      /* TODO */
      Orly::Package::TManager PackageManager;

      /* This is necessary to make the type singletons and interners visible to the packages being loaded. */
      Type::TTypeCzar TypeCzar;

      /* The scheduler we use to launch jobs.  Set by the constructor and never changed. */
      Base::TScheduler *const Scheduler;

      /* The command-line arguments we constructed with. */
      const TCmd &Cmd;

      /* The manager of durable objects.  Set by the constructor and never null. */
      std::shared_ptr<Durable::TManager> DurableManager;

      /* Tetris lives here. */
      TRepoTetrisManager *TetrisManager;

      /* The timer waited for by CleanHouse(). */
      Base::TTimerFd HousecleaningTimer;

      /* The socket on which AcceptClientConnections() listens. */
      Base::TFd MainSocket;

      /* The socket on which we listen for memcached clients. */
      Base::TFd MemcacheSocket;

      /* Covers ConnectionBySessionId. */
      std::mutex ConnectionMutex;

      /* All our open connections. */
      std::unordered_map<Base::TUuid, std::weak_ptr<TConnection>> ConnectionBySessionId;

      /* TODO */
      std::unique_ptr<TIndyReporter> Reporter;

      /* TODO */
      std::shared_ptr<Orly::Indy::Disk::TIndyUtilReporter> UtilReporter;

      /* TODO */
      std::shared_ptr<std::function<void (const Base::TFd &)>> WaitForSlaveActionCb;

      /* TODO */
      bool InitFinished;
      std::condition_variable InitCond;
      std::mutex InitMutex;

      /* The websockets server object */
      std::unique_ptr<TWs> Ws;

      /* The thread on which WsRunner runs. */
      std::thread WsThread;

      /* TODO */
      friend class TIndyReporter;

    };  // TServer

  }  // Server

}  // Orly
