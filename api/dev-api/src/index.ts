import express from 'express';
import cors from 'cors';
import helmet from 'helmet';
import morgan from 'morgan';
import compression from 'compression';
import rateLimit from 'express-rate-limit';
import dotenv from 'dotenv';

// Import routes
import authRoutes from './routes/auth';
import apiKeyRoutes from './routes/apiKeys';
import usageRoutes from './routes/usage';
import statsRoutes from './routes/stats';
import registryRoutes from './routes/registry';
import verifiedRoutes from './routes/verified';

// Import middleware
import { errorHandler } from './middleware/errorHandler';
import { authMiddleware } from './middleware/auth';
import { rateLimitMiddleware } from './middleware/rateLimit';

// Import database connection
import { connectDatabase } from './config/database';

// Load environment variables
dotenv.config();

const app = express();
const PORT = process.env.PORT || 3001;

// Security middleware
app.use(helmet());
app.use(cors({
  origin: process.env.ALLOWED_ORIGINS?.split(',') || ['http://localhost:3000'],
  credentials: true
}));

// Compression middleware
app.use(compression());

// Logging middleware
app.use(morgan('combined'));

// Body parsing middleware
app.use(express.json({ limit: '10mb' }));
app.use(express.urlencoded({ extended: true, limit: '10mb' }));

// Rate limiting
const limiter = rateLimit({
  windowMs: 15 * 60 * 1000, // 15 minutes
  max: 100, // limit each IP to 100 requests per windowMs
  message: 'Too many requests from this IP, please try again later.',
  standardHeaders: true,
  legacyHeaders: false,
});
app.use(limiter);

// Health check endpoint
app.get('/health', (req, res) => {
  res.status(200).json({
    status: 'OK',
    timestamp: new Date().toISOString(),
    uptime: process.uptime(),
    environment: process.env.NODE_ENV || 'development'
  });
});

// API routes
app.use('/api/auth', authRoutes);
app.use('/api/keys', authMiddleware, apiKeyRoutes);
app.use('/api/usage', authMiddleware, usageRoutes);
app.use('/api/stats', statsRoutes);
app.use('/api/registry', registryRoutes);
app.use('/api/verified', verifiedRoutes);

// API documentation endpoint
app.get('/api/docs', (req, res) => {
  res.json({
    name: 'SHAHCOIN Developer API',
    version: '1.0.0',
    description: 'API for SHAHCOIN developer tools and services',
    endpoints: {
      auth: {
        'POST /api/auth/register': 'Register a new developer account',
        'POST /api/auth/login': 'Login with email/password',
        'POST /api/auth/wallet': 'Login with wallet signature',
        'POST /api/auth/refresh': 'Refresh access token',
        'POST /api/auth/logout': 'Logout and invalidate tokens'
      },
      apiKeys: {
        'GET /api/keys': 'Get all API keys for the authenticated developer',
        'POST /api/keys': 'Create a new API key',
        'DELETE /api/keys/:id': 'Revoke an API key',
        'PUT /api/keys/:id': 'Update API key permissions'
      },
      usage: {
        'POST /api/usage': 'Log API usage for tracking',
        'GET /api/usage': 'Get usage statistics for the developer',
        'GET /api/usage/analytics': 'Get detailed usage analytics'
      },
      stats: {
        'GET /api/stats': 'Get global platform statistics',
        'GET /api/stats/developers': 'Get developer statistics',
        'GET /api/stats/plugins': 'Get plugin statistics'
      },
      registry: {
        'GET /api/registry': 'Get all plugins in the registry',
        'POST /api/registry': 'Submit a new plugin',
        'GET /api/registry/:id': 'Get plugin details',
        'PUT /api/registry/:id': 'Update plugin information',
        'DELETE /api/registry/:id': 'Remove plugin from registry'
      },
      verified: {
        'GET /api/verified': 'Get verified developer status',
        'POST /api/verified/apply': 'Apply for verified developer status',
        'GET /api/verified/badge': 'Get verified developer badge'
      }
    },
    authentication: {
      type: 'Bearer Token',
      header: 'Authorization: Bearer <token>'
    },
    rateLimiting: {
      limit: '100 requests per 15 minutes per IP',
      headers: 'X-RateLimit-* headers included'
    }
  });
});

// 404 handler
app.use('*', (req, res) => {
  res.status(404).json({
    error: 'Endpoint not found',
    message: `The requested endpoint ${req.originalUrl} does not exist`,
    availableEndpoints: '/api/docs'
  });
});

// Error handling middleware
app.use(errorHandler);

// Start server
async function startServer() {
  try {
    // Connect to database
    await connectDatabase();
    console.log('✅ Database connected successfully');

    // Start server
    app.listen(PORT, () => {
      console.log(`🚀 SHAHCOIN Developer API Gateway running on port ${PORT}`);
      console.log(`📚 API Documentation: http://localhost:${PORT}/api/docs`);
      console.log(`🏥 Health Check: http://localhost:${PORT}/health`);
      console.log(`🌍 Environment: ${process.env.NODE_ENV || 'development'}`);
    });
  } catch (error) {
    console.error('❌ Failed to start server:', error);
    process.exit(1);
  }
}

// Graceful shutdown
process.on('SIGTERM', () => {
  console.log('🛑 SIGTERM received, shutting down gracefully');
  process.exit(0);
});

process.on('SIGINT', () => {
  console.log('🛑 SIGINT received, shutting down gracefully');
  process.exit(0);
});

// Handle uncaught exceptions
process.on('uncaughtException', (error) => {
  console.error('❌ Uncaught Exception:', error);
  process.exit(1);
});

process.on('unhandledRejection', (reason, promise) => {
  console.error('❌ Unhandled Rejection at:', promise, 'reason:', reason);
  process.exit(1);
});

// Start the server
startServer();
