import React from 'react';
import { BadgeDetectionSystem, BADGE_DEFINITIONS } from './metadata-standards.js';
import './badge-styles.css';

/**
 * TokenBadge Component
 * 
 * Displays SHI-20 and SHI-30 token badges with proper styling and tooltips
 */
class TokenBadge extends React.Component {
  constructor(props) {
    super(props);
    this.badgeSystem = new BadgeDetectionSystem();
    this.state = {
      badge: null,
      loading: true,
      error: null
    };
  }

  componentDidMount() {
    this.detectBadge();
  }

  componentDidUpdate(prevProps) {
    if (prevProps.metadata !== this.props.metadata || 
        prevProps.contractAddress !== this.props.contractAddress) {
      this.detectBadge();
    }
  }

  detectBadge() {
    try {
      const { metadata, contractAddress } = this.props;
      
      if (!metadata && !contractAddress) {
        this.setState({
          badge: BADGE_DEFINITIONS.UNKNOWN,
          loading: false
        });
        return;
      }

      // If we have metadata, use it directly
      if (metadata) {
        const badge = this.badgeSystem.detectTokenBadge(metadata);
        this.setState({
          badge,
          loading: false
        });
        return;
      }

      // If we only have contract address, try to fetch metadata
      if (contractAddress) {
        this.fetchMetadataFromAddress(contractAddress);
      }
    } catch (error) {
      console.error('Error detecting badge:', error);
      this.setState({
        badge: BADGE_DEFINITIONS.UNKNOWN,
        loading: false,
        error: error.message
      });
    }
  }

  async fetchMetadataFromAddress(contractAddress) {
    try {
      this.setState({ loading: true });
      
      // This would typically fetch from blockchain or API
      // For now, we'll simulate with a timeout
      await new Promise(resolve => setTimeout(resolve, 100));
      
      // Mock metadata fetch - in real implementation, this would call an API
      const mockMetadata = await this.getMockMetadata(contractAddress);
      
      const badge = this.badgeSystem.detectTokenBadge(mockMetadata);
      this.setState({
        badge,
        loading: false
      });
    } catch (error) {
      console.error('Error fetching metadata:', error);
      this.setState({
        badge: BADGE_DEFINITIONS.UNKNOWN,
        loading: false,
        error: error.message
      });
    }
  }

  async getMockMetadata(contractAddress) {
    // Mock implementation - in real app, this would fetch from blockchain
    // or Shahcoin metadata API
    const mockData = {
      '0x1234567890123456789012345678901234567890': {
        standard: 'shahcoin.token.standard.v1',
        type: 'SHI-20',
        name: 'Mock Token',
        symbol: 'MTK',
        createdBy: 'ShahFactory',
        verified: false
      },
      '0x0987654321098765432109876543210987654321': {
        standard: 'shahcoin.nft.standard.v1',
        type: 'SHI-30',
        name: 'Mock NFT',
        symbol: 'MNFT',
        createdBy: 'ShahFactory',
        verified: true
      }
    };

    return mockData[contractAddress] || null;
  }

  getBadgeClasses() {
    const { badge, loading } = this.state;
    const { size = 'medium', className = '' } = this.props;

    if (loading) {
      return `shahcoin-badge badge-loading badge-${size} ${className}`.trim();
    }

    if (!badge) {
      return `shahcoin-badge badge-unknown badge-${size} ${className}`.trim();
    }

    const baseClasses = `shahcoin-badge badge-${size}`;
    const badgeClasses = this.badgeSystem.getBadgeCSS(badge);
    const badgeClassNames = Object.keys(badgeClasses).filter(key => badgeClasses[key]).join(' ');
    
    return `${baseClasses} ${badgeClassNames} ${className}`.trim();
  }

  renderBadgeContent() {
    const { badge, loading } = this.state;
    const { showLabel = true, showIcon = true } = this.props;

    if (loading) {
      return (
        <>
          {showIcon && <span className="badge-icon">⏳</span>}
          {showLabel && <span className="badge-label">Loading...</span>}
        </>
      );
    }

    if (!badge) {
      return (
        <>
          {showIcon && <span className="badge-icon">{BADGE_DEFINITIONS.UNKNOWN.icon}</span>}
          {showLabel && <span className="badge-label">{BADGE_DEFINITIONS.UNKNOWN.label}</span>}
        </>
      );
    }

    return (
      <>
        {showIcon && <span className="badge-icon">{badge.icon}</span>}
        {showLabel && <span className="badge-label">{badge.label}</span>}
      </>
    );
  }

  render() {
    const { badge, loading, error } = this.state;
    const { 
      onClick, 
      title, 
      'aria-label': ariaLabel,
      tabIndex = 0,
      role = 'button'
    } = this.props;

    const badgeClasses = this.getBadgeClasses();
    const badgeTitle = title || (badge ? badge.tooltip : 'Unknown token type');
    const badgeAriaLabel = ariaLabel || (badge ? badge.label : 'Unknown token');

    return (
      <span
        className={badgeClasses}
        title={badgeTitle}
        aria-label={badgeAriaLabel}
        tabIndex={onClick ? tabIndex : undefined}
        role={onClick ? role : undefined}
        onClick={onClick}
        onKeyDown={(e) => {
          if (onClick && (e.key === 'Enter' || e.key === ' ')) {
            e.preventDefault();
            onClick(e);
          }
        }}
        style={{
          cursor: onClick ? 'pointer' : 'help'
        }}
      >
        {this.renderBadgeContent()}
        {error && (
          <span className="sr-only" role="alert">
            Error: {error}
          </span>
        )}
      </span>
    );
  }
}

/**
 * TokenBadgeGroup Component
 * 
 * Displays multiple badges together (e.g., SHI-20 + Verified)
 */
class TokenBadgeGroup extends React.Component {
  render() {
    const { badges, className = '', size = 'medium' } = this.props;

    return (
      <div className={`badge-group ${className}`.trim()}>
        {badges.map((badgeProps, index) => (
          <TokenBadge
            key={index}
            size={size}
            {...badgeProps}
          />
        ))}
      </div>
    );
  }
}

/**
 * TokenBadgeWithInfo Component
 * 
 * Displays a badge with additional information on click/hover
 */
class TokenBadgeWithInfo extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      showInfo: false
    };
  }

  toggleInfo = () => {
    this.setState(prevState => ({
      showInfo: !prevState.showInfo
    }));
  }

  render() {
    const { showInfo } = this.state;
    const { 
      metadata, 
      contractAddress, 
      infoContent,
      className = '',
      ...badgeProps 
    } = this.props;

    return (
      <div className={`badge-with-info-container ${className}`.trim()}>
        <TokenBadge
          {...badgeProps}
          metadata={metadata}
          contractAddress={contractAddress}
          onClick={this.toggleInfo}
          className={`badge-with-info ${badgeProps.className || ''}`.trim()}
        />
        {showInfo && infoContent && (
          <div className="badge-info-popup">
            {infoContent}
          </div>
        )}
      </div>
    );
  }
}

/**
 * TokenBadgeList Component
 * 
 * Displays a list of tokens with their badges
 */
class TokenBadgeList extends React.Component {
  render() {
    const { tokens, className = '', size = 'medium' } = this.props;

    return (
      <div className={`token-badge-list ${className}`.trim()}>
        {tokens.map((token, index) => (
          <div key={index} className="token-badge-item">
            <span className="token-name">{token.name}</span>
            <TokenBadge
              metadata={token.metadata}
              contractAddress={token.contractAddress}
              size={size}
            />
          </div>
        ))}
      </div>
    );
  }
}

// Export components
export {
  TokenBadge,
  TokenBadgeGroup,
  TokenBadgeWithInfo,
  TokenBadgeList
};

export default TokenBadge;
