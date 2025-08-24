import { useState, useEffect } from 'react';
import { motion } from 'framer-motion';
import { 
  ChartBarIcon, 
  CodeBracketIcon, 
  KeyIcon, 
  PuzzlePieceIcon,
  UserGroupIcon,
  DocumentTextIcon,
  RocketLaunchIcon,
  ShieldCheckIcon
} from '@heroicons/react/24/outline';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, BarChart, Bar } from 'recharts';
import { format } from 'date-fns';

interface DashboardStats {
  totalDevelopers: number;
  activeProjects: number;
  apiCallsToday: number;
  totalDownloads: number;
  verifiedDevelopers: number;
  pluginsPublished: number;
}

interface UsageData {
  date: string;
  apiCalls: number;
  downloads: number;
}

export default function DeveloperPortal() {
  const [stats, setStats] = useState<DashboardStats>({
    totalDevelopers: 0,
    activeProjects: 0,
    apiCallsToday: 0,
    totalDownloads: 0,
    verifiedDevelopers: 0,
    pluginsPublished: 0
  });

  const [usageData, setUsageData] = useState<UsageData[]>([]);
  const [isLoading, setIsLoading] = useState(true);

  useEffect(() => {
    // Simulate loading dashboard data
    const loadDashboardData = async () => {
      setIsLoading(true);
      
      // Simulate API call delay
      await new Promise(resolve => setTimeout(resolve, 1000));
      
      // Mock data
      setStats({
        totalDevelopers: 1247,
        activeProjects: 89,
        apiCallsToday: 15420,
        totalDownloads: 45678,
        verifiedDevelopers: 234,
        pluginsPublished: 67
      });

      // Mock usage data for the last 7 days
      const mockUsageData: UsageData[] = Array.from({ length: 7 }, (_, i) => ({
        date: format(new Date(Date.now() - (6 - i) * 24 * 60 * 60 * 1000), 'MMM dd'),
        apiCalls: Math.floor(Math.random() * 5000) + 10000,
        downloads: Math.floor(Math.random() * 1000) + 500
      }));

      setUsageData(mockUsageData);
      setIsLoading(false);
    };

    loadDashboardData();
  }, []);

  const statCards = [
    {
      title: 'Total Developers',
      value: stats.totalDevelopers.toLocaleString(),
      icon: UserGroupIcon,
      color: 'bg-blue-500',
      change: '+12%',
      changeType: 'positive'
    },
    {
      title: 'Active Projects',
      value: stats.activeProjects.toLocaleString(),
      icon: CodeBracketIcon,
      color: 'bg-green-500',
      change: '+8%',
      changeType: 'positive'
    },
    {
      title: 'API Calls Today',
      value: stats.apiCallsToday.toLocaleString(),
      icon: ChartBarIcon,
      color: 'bg-purple-500',
      change: '+23%',
      changeType: 'positive'
    },
    {
      title: 'Total Downloads',
      value: stats.totalDownloads.toLocaleString(),
      icon: RocketLaunchIcon,
      color: 'bg-orange-500',
      change: '+15%',
      changeType: 'positive'
    },
    {
      title: 'Verified Developers',
      value: stats.verifiedDevelopers.toLocaleString(),
      icon: ShieldCheckIcon,
      color: 'bg-emerald-500',
      change: '+5%',
      changeType: 'positive'
    },
    {
      title: 'Plugins Published',
      value: stats.pluginsPublished.toLocaleString(),
      icon: PuzzlePieceIcon,
      color: 'bg-pink-500',
      change: '+18%',
      changeType: 'positive'
    }
  ];

  const quickActions = [
    {
      title: 'Download SDK',
      description: 'Get the latest SHAHCOIN SDK',
      icon: CodeBracketIcon,
      href: '/sdk',
      color: 'bg-blue-500 hover:bg-blue-600'
    },
    {
      title: 'API Keys',
      description: 'Manage your API keys',
      icon: KeyIcon,
      href: '/api-keys',
      color: 'bg-green-500 hover:bg-green-600'
    },
    {
      title: 'Plugin Registry',
      description: 'Browse and publish plugins',
      icon: PuzzlePieceIcon,
      href: '/registry',
      color: 'bg-purple-500 hover:bg-purple-600'
    },
    {
      title: 'Documentation',
      description: 'Read the latest docs',
      icon: DocumentTextIcon,
      href: '/docs',
      color: 'bg-orange-500 hover:bg-orange-600'
    }
  ];

  if (isLoading) {
    return (
      <div className="min-h-screen bg-gray-50 flex items-center justify-center">
        <div className="animate-spin rounded-full h-32 w-32 border-b-2 border-blue-500"></div>
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-gray-50">
      {/* Header */}
      <div className="bg-white shadow">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="flex justify-between items-center py-6">
            <div>
              <h1 className="text-3xl font-bold text-gray-900">SHAHCOIN Developer Portal</h1>
              <p className="mt-1 text-sm text-gray-500">
                Welcome back! Here's what's happening with your development tools.
              </p>
            </div>
            <div className="flex items-center space-x-4">
              <button className="bg-blue-500 hover:bg-blue-600 text-white px-4 py-2 rounded-lg text-sm font-medium">
                Get Started
              </button>
            </div>
          </div>
        </div>
      </div>

      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        {/* Stats Grid */}
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6 mb-8">
          {statCards.map((stat, index) => (
            <motion.div
              key={stat.title}
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ delay: index * 0.1 }}
              className="bg-white rounded-lg shadow p-6"
            >
              <div className="flex items-center">
                <div className={`p-3 rounded-lg ${stat.color}`}>
                  <stat.icon className="h-6 w-6 text-white" />
                </div>
                <div className="ml-4 flex-1">
                  <p className="text-sm font-medium text-gray-500">{stat.title}</p>
                  <p className="text-2xl font-semibold text-gray-900">{stat.value}</p>
                </div>
                <div className={`text-sm font-medium ${
                  stat.changeType === 'positive' ? 'text-green-600' : 'text-red-600'
                }`}>
                  {stat.change}
                </div>
              </div>
            </motion.div>
          ))}
        </div>

        {/* Charts Section */}
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-8 mb-8">
          {/* API Usage Chart */}
          <motion.div
            initial={{ opacity: 0, x: -20 }}
            animate={{ opacity: 1, x: 0 }}
            transition={{ delay: 0.3 }}
            className="bg-white rounded-lg shadow p-6"
          >
            <h3 className="text-lg font-medium text-gray-900 mb-4">API Usage (Last 7 Days)</h3>
            <ResponsiveContainer width="100%" height={300}>
              <LineChart data={usageData}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="date" />
                <YAxis />
                <Tooltip />
                <Line type="monotone" dataKey="apiCalls" stroke="#3B82F6" strokeWidth={2} />
              </LineChart>
            </ResponsiveContainer>
          </motion.div>

          {/* Downloads Chart */}
          <motion.div
            initial={{ opacity: 0, x: 20 }}
            animate={{ opacity: 1, x: 0 }}
            transition={{ delay: 0.4 }}
            className="bg-white rounded-lg shadow p-6"
          >
            <h3 className="text-lg font-medium text-gray-900 mb-4">SDK Downloads (Last 7 Days)</h3>
            <ResponsiveContainer width="100%" height={300}>
              <BarChart data={usageData}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="date" />
                <YAxis />
                <Tooltip />
                <Bar dataKey="downloads" fill="#10B981" />
              </BarChart>
            </ResponsiveContainer>
          </motion.div>
        </div>

        {/* Quick Actions */}
        <motion.div
          initial={{ opacity: 0, y: 20 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ delay: 0.5 }}
          className="bg-white rounded-lg shadow p-6"
        >
          <h3 className="text-lg font-medium text-gray-900 mb-6">Quick Actions</h3>
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
            {quickActions.map((action) => (
              <a
                key={action.title}
                href={action.href}
                className={`${action.color} text-white p-4 rounded-lg transition-colors duration-200`}
              >
                <div className="flex items-center">
                  <action.icon className="h-8 w-8" />
                  <div className="ml-3">
                    <h4 className="font-medium">{action.title}</h4>
                    <p className="text-sm opacity-90">{action.description}</p>
                  </div>
                </div>
              </a>
            ))}
          </div>
        </motion.div>

        {/* Recent Activity */}
        <motion.div
          initial={{ opacity: 0, y: 20 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ delay: 0.6 }}
          className="bg-white rounded-lg shadow p-6 mt-8"
        >
          <h3 className="text-lg font-medium text-gray-900 mb-6">Recent Activity</h3>
          <div className="space-y-4">
            {[
              { action: 'New plugin published', user: 'alice_dev', time: '2 hours ago', type: 'plugin' },
              { action: 'API key generated', user: 'bob_developer', time: '4 hours ago', type: 'key' },
              { action: 'SDK downloaded', user: 'charlie_coder', time: '6 hours ago', type: 'download' },
              { action: 'Developer verified', user: 'diana_creator', time: '1 day ago', type: 'verify' }
            ].map((activity, index) => (
              <div key={index} className="flex items-center space-x-4 p-3 bg-gray-50 rounded-lg">
                <div className={`p-2 rounded-full ${
                  activity.type === 'plugin' ? 'bg-purple-100 text-purple-600' :
                  activity.type === 'key' ? 'bg-green-100 text-green-600' :
                  activity.type === 'download' ? 'bg-blue-100 text-blue-600' :
                  'bg-emerald-100 text-emerald-600'
                }`}>
                  {activity.type === 'plugin' && <PuzzlePieceIcon className="h-4 w-4" />}
                  {activity.type === 'key' && <KeyIcon className="h-4 w-4" />}
                  {activity.type === 'download' && <RocketLaunchIcon className="h-4 w-4" />}
                  {activity.type === 'verify' && <ShieldCheckIcon className="h-4 w-4" />}
                </div>
                <div className="flex-1">
                  <p className="text-sm font-medium text-gray-900">{activity.action}</p>
                  <p className="text-sm text-gray-500">by {activity.user}</p>
                </div>
                <span className="text-sm text-gray-400">{activity.time}</span>
              </div>
            ))}
          </div>
        </motion.div>
      </div>
    </div>
  );
}
