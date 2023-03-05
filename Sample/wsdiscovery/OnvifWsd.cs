//#define DEBUG_WSD
//#define DEBUG_CONSOLE
#define STUPIDCAM

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using System.ServiceModel;
using System.ServiceModel.Discovery;
using System.Xml;
using System.Diagnostics;
using System.CodeDom;
using System.Runtime.CompilerServices;

namespace OnvifDiscoveryDotnet
{
    public static class Dbg
    {
        public static void WriteLine(this string str, params object[] args)
        {
#if DEBUG_WSD
#if DEBUG_CONSOLE
            Console.WriteLine(str, args);
#else
            Debug.WriteLine(str, args);
#endif
#endif
        }
    }

    public class OnvifWsd
    {
        /* announcement service */
        private ServiceHost announceServiceHost_;
        private AnnouncementService announceService_;
        private UdpAnnouncementEndpoint announceEp_;

        /* prove&resolve client */
        private DiscoveryClient discoveryClient_;
        private UdpDiscoveryEndpoint discoveryEp_;

        private FindCriteria findCriteria_;
        private ResolveCriteria resolveCriteria_;

        /* discovery parameters */
        private Dictionary<EndpointAddress, ResolvedEndpoint> services_;
        public Dictionary<EndpointAddress, ResolvedEndpoint> CurrentServices 
        { 
            get 
            {
#if (false)
                lock(services_)
                {
                    var copy = new Dictionary<EndpointAddress, ResolvedEndpoint>(services_);
                    return copy;
                }
#else
                return services_;
#endif
            } 
        } 

        public List<ResolvedEndpoint1> CurrentServices1
        {
            get
            {
                List<ResolvedEndpoint1> ret = new List<ResolvedEndpoint1>();
                int idx;
                lock (services_)
                {
                    foreach (var n in services_)
                    {
                        var ep = new ResolvedEndpoint1
                        {
                            uuid = n.Key.Uri.ToString(),
                            xaddr = n.Value.metadata.ListenUris[0].ToString(),
                            name = ""
                        };

                        foreach (var scope in n.Value.metadata.Scopes)
                        {
                            idx = scope.ToString().IndexOf(@"name/");
                            if (idx != -1)
                            {
                                ep.name = scope.ToString().Substring(idx + 5);
                                break;
                            }
                        }
                        ret.Add(ep);
                    }
                }
                return ret;
            }
        }

        private bool bStarted_ = false;
        public bool IsRunning { get { return bStarted_; } }

        private readonly DiscoveryVersion version_ = DiscoveryVersion.WSDiscoveryApril2005;
        public DiscoveryVersion DiscoveryVersion { get { return version_; } }

        private readonly XmlQualifiedName qname_;
        public XmlQualifiedName Typenamespace { get { return qname_; } }

        private readonly double probeDurationMs_ = 3000;
        private readonly double resolveDurationMs_ = 1000;
        private readonly int updateDelayMs_ = 10000;
        private readonly int updateCap_ = 15;
        
        private CancellationTokenSource cts_ = null;
        private readonly Random rand_ = new Random();

        public OnvifWsd()
        {
            qname_ = new XmlQualifiedName("NetworkVideoTransmitter", @"http://www.onvif.org/ver10/network/wsdl");
            InitializeServices();
        }

        public OnvifWsd(DiscoveryVersion version, XmlQualifiedName qname)
        {
            version_ = version;
            qname_ = qname;
            InitializeServices();
        }

        ~OnvifWsd()
        {
            Stop();
        }

        private void InitializeServices()
        {
            /* announcement */
            services_ = new Dictionary<EndpointAddress, ResolvedEndpoint>();

            announceEp_ = new UdpAnnouncementEndpoint(version_);
            announceService_ = new AnnouncementService();
            announceService_.OnlineAnnouncementReceived += OnHello;
            announceService_.OfflineAnnouncementReceived += OnBye;

            announceServiceHost_ = new ServiceHost(announceService_);
            announceServiceHost_.AddServiceEndpoint(announceEp_);

            /* prove&resolve */
            discoveryEp_ = new UdpDiscoveryEndpoint(version_);
#pragma warning disable CS0618 // Type or member is obsolete
            // RSJ : 여러번 보내면 여러번 응답이 오고 그만큼 이벤트가 발생한다. 이상한데?
            discoveryEp_.TransportSettings.MaxMulticastRetransmitCount = 0;
#pragma warning restore CS0618 // Type or member is obsolete

            discoveryClient_ = new DiscoveryClient(discoveryEp_);
            discoveryClient_.ResolveCompleted += ResolveCompleted;
            //stupid cheap cam is not able to handle resolve message, so...
            //discoveryClient_.FindCompleted += FindCompleted;
            //discoveryClient_.FindProgressChanged += FindProgressChanged;

            findCriteria_ = new FindCriteria
            {
                Duration = TimeSpan.FromMilliseconds(probeDurationMs_),
            };
            findCriteria_.ContractTypeNames.Add(qname_);

            findCriteria_.ScopeMatchBy = new Uri("http://schemas.xmlsoap.org/ws/2005/04/discovery/rfc3986");
            //findCriteria_.Scopes.Add(new Uri("onvif://www.onvif.org/type/video_encoder"));
            //findCriteria_.Scopes.Add(new Uri("onvif://www.onvif.org/"));

            resolveCriteria_ = new ResolveCriteria
            {
                Duration = TimeSpan.FromMilliseconds(resolveDurationMs_)
            };

            cts_ = new CancellationTokenSource();
        }

        public void Start()
        {
            if (bStarted_ == false)
            {
                bStarted_ = true;
                ServiceT();
            }
        }

        private async void ServiceT()
        {
            CancellationTokenSource cts = cts_;

            try
            {
                await Task.Run(async () => 
                {
#if STUPIDCAM
                    do
                    {
                        var probe = discoveryClient_.FindTaskAsync(findCriteria_, cts.Token);
                        probe.Wait(cts.Token);
                        "probe finished".WriteLine();
                        var response = probe.Result;
                        lock (services_)
                        {
                            services_.Clear();
                            foreach (var ep in response.Endpoints)
                            {
                                var re = new ResolvedEndpoint(ep, rand_.Next(updateCap_));
                                services_[ep.Address] = re;
                                "Received an probe response from {0}:".WriteLine(ep.Address);
                            }
                        }
                        await Task.Delay(updateDelayMs_, cts.Token);
                    } while (true);
#else
                    "start probe".WriteLine();
                    var probe = discoveryClient_.FindTaskAsync(findCriteria_, cts.Token);
                    probe.Wait(cts.Token);
                    "probe finished".WriteLine();

                    var response = probe.Result;
                    lock (services_)
                    {
                        foreach (var ep in response.Endpoints)
                        {
                            var re = new ResolvedEndpoint(ep, rand_.Next(updateCap_));
                            services_[ep.Address] = re;
                            "Received an probe response from {0}:".WriteLine(ep.Address);
                        }
                    }
                    announceServiceHost_.Open();

                    int cnt = 0;
                    while (true)
                    {
                        await Task.Delay(updateDelayMs_, cts.Token);

                        lock (services_)
                        {
                            foreach (var ep in services_)
                            {
                                if (ep.Value.cnt == cnt)
                                {
                                    //resolveCriteria_.Address = new EndpointAddress(ep.Value.metadata.ListenUris[0]);
                                    //discoveryClient_.ResolveAsync(resolveCriteria_, ep.Value.metadata.Address);
                                    resolveCriteria_.Address = ep.Value.metadata.Address;
                                    discoveryClient_.ResolveAsync(resolveCriteria_, resolveCriteria_.Address);
                                    "Start resolving {0}".WriteLine(resolveCriteria_.Address);
                                }
                                cnt = (cnt + 1) % updateCap_;
                            }
                        }
                    }
#endif
                }, cts.Token);
            }
            catch (OperationCanceledException ce)
            {
                "discovery service is terminated by {0}".WriteLine(ce.Message);
            }
            finally
            {
                announceServiceHost_.Close();
                discoveryClient_.Close();
            }
        }

        public void Stop()
        {
            if (bStarted_)
            {
                cts_.Cancel();
                bStarted_ = false;
            }
        }

        public void Clear()
        {
            lock (services_)
            {
                services_.Clear();
            }
        }

        // announcement
        private void OnHello(object sender, AnnouncementEventArgs e)
        {
            lock (services_)
            {
                var metadata = e.EndpointDiscoveryMetadata;

                foreach (var qname in metadata.ContractTypeNames)
                {
                    if (qname == qname_)
                    {
                        var re = new ResolvedEndpoint(metadata, rand_.Next(updateCap_));
                        services_[metadata.Address] = re;

                        "Received an online announcement from {0}:".WriteLine(metadata.Address);
                        break;
                    }
                }
            }
        }

        private void OnBye(object sender, AnnouncementEventArgs e)
        {
            lock (services_)
            {
                var metadata = e.EndpointDiscoveryMetadata;

                foreach (var qname in metadata.ContractTypeNames)
                {
                    if (qname == qname_)
                    {
                        services_.Remove(metadata.Address);

                        "Received an offline announcement from {0}:".WriteLine(metadata.Address);
                        break;
                    }
                }
            }
        }

        private void ResolveCompleted(object sender, ResolveCompletedEventArgs e)
        {
            lock (services_)
            {
                EndpointAddress addr = (EndpointAddress)e.UserState;

                if (e.Result.EndpointDiscoveryMetadata == null)
                {
                    services_.Remove(addr);

                    "resolve failed {0}".WriteLine(addr);
                }
                else
                {
                    "resolve succeeded {0}".WriteLine(addr);
                }
            }
        }

        private void FindCompleted(object sender, FindCompletedEventArgs e) 
        {
            lock (services_)
            {
                foreach (var ep in e.Result.Endpoints)
                {
                    var re = new ResolvedEndpoint(ep, rand_.Next(updateCap_));
                    services_[ep.Address] = re;
                    "Received an probe response from {0}:".WriteLine(ep.Address);
                }
            }
            bStarted_ = false;
        }

        private void FindProgressChanged(object sender, FindProgressChangedEventArgs e)
        {
            "progress changed".WriteLine();
            lock (services_)
            {
                var ep = e.EndpointDiscoveryMetadata;
                if (ep != null)
                {
                    var re = new ResolvedEndpoint(ep, rand_.Next(updateCap_));
                    services_[ep.Address] = re;
                    "Received an probe response from {0}:".WriteLine(ep.Address);
                }
            }
        }
    }

    public struct ResolvedEndpoint
    {
        public ResolvedEndpoint(EndpointDiscoveryMetadata m, int c)
        {
            metadata = m;
            cnt = c;
        }

        public EndpointDiscoveryMetadata metadata;
        public int cnt;
    }

    public struct ResolvedEndpoint1
    {
        public String uuid;
        public String xaddr;
        public String name;
    }
}
