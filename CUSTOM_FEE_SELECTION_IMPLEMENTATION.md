# Custom Fee Selection UI Implementation

## Overview

The Custom Fee Selection UI provides users with both Basic Mode (Standard, High Priority, Eco Mode) and Advanced Mode (custom fee per kilobyte) options for transaction fee control in the Shahcoin Qt Wallet. This implementation enhances the existing fee system with a user-friendly interface that caters to both novice and advanced users.

## Features

### Basic Mode
- **Standard Fee**: Default fee for normal transaction priority (~25 blocks)
- **High Priority**: Higher fee for faster confirmation (~6 blocks)
- **Eco Mode**: Lower fee for slower confirmation, cheaper (~100 blocks)
- **Real-time Estimates**: Shows estimated fee and confirmation time for each mode

### Advanced Mode
- **Custom Fee Input**: Direct input of fee per kilobyte (satoshis/kB)
- **Confirmation Target**: Selectable confirmation target (1-100 blocks)
- **Transaction Size Display**: Shows estimated transaction size in bytes
- **Total Fee Calculation**: Real-time calculation of total transaction fee
- **Fee Validation**: Warns if fee is below minimum relay fee

### User Experience
- **Toggle Between Modes**: Easy switch between Basic and Advanced modes
- **Persistent Settings**: Remembers user preferences across sessions
- **Reset to Defaults**: Quick reset to standard settings
- **Visual Feedback**: Color-coded warnings and real-time updates

## Technical Implementation

### Files Created/Modified

#### New Files
- `src/qt/customfeeselectiondialog.h`: Header file defining the CustomFeeSelectionDialog class
- `src/qt/customfeeselectiondialog.cpp`: Implementation of the custom fee selection dialog
- `CUSTOM_FEE_SELECTION_IMPLEMENTATION.md`: This documentation file

#### Modified Files
- `src/qt/sendcoinsdialog.h`: Added include and method declaration for custom fee selection
- `src/qt/sendcoinsdialog.cpp`: Added custom fee button and integration logic
- `src/Makefile.qt.include`: Added new files to build system

### Class Structure

#### CustomFeeSelectionDialog
```cpp
class CustomFeeSelectionDialog : public QDialog
{
public:
    enum FeeMode {
        Standard,      // Standard fee (default)
        HighPriority,  // High priority (faster, more expensive)
        EcoMode,       // Eco mode (slower, cheaper)
        Custom         // Custom fee per kilobyte
    };
    
    // Getter methods
    FeeMode getSelectedMode() const;
    CAmount getCustomFeePerKB() const;
    int getConfirmationTarget() const;
    
    // Setter methods
    void setCurrentMode(FeeMode mode);
    void setCurrentCustomFee(CAmount feePerKB);
    void setCurrentConfirmationTarget(int target);
};
```

### UI Components

#### Basic Mode Group
- **Radio Buttons**: Standard, High Priority, Eco Mode selection
- **Information Labels**: Estimated fee and confirmation time for each mode
- **Real-time Updates**: Dynamic fee calculation based on network conditions

#### Advanced Mode Group
- **Custom Fee Radio Button**: Toggle for custom fee mode
- **Fee Input Field**: SpinBox for fee per kilobyte input
- **Confirmation Target Combo**: Dropdown for block confirmation target
- **Information Display**: Transaction size, total fee, confirmation time
- **Warning Labels**: Visual warnings for low fees

#### Control Buttons
- **Reset to Defaults**: Restore standard settings
- **OK/Cancel**: Standard dialog controls

### Fee Calculation Logic

#### Basic Mode Fee Estimation
```cpp
// Standard fee (25 blocks)
CFeeRate standardRate = CFeeRate(model->wallet().getMinimumFee(1000, *m_coin_control, &returned_target, &reason));
m_standardFeeRate = standardRate.GetFeePerK();

// High priority fee (6 blocks)
m_coin_control->m_confirm_target = 6;
CFeeRate highPriorityRate = CFeeRate(model->wallet().getMinimumFee(1000, *m_coin_control, &returned_target, &reason));
m_highPriorityFeeRate = highPriorityRate.GetFeePerK();

// Eco mode fee (100 blocks)
m_coin_control->m_confirm_target = 100;
CFeeRate ecoRate = CFeeRate(model->wallet().getMinimumFee(1000, *m_coin_control, &returned_target, &reason));
m_ecoModeFeeRate = ecoRate.GetFeePerK();
```

#### Custom Fee Calculation
```cpp
CAmount totalFee = (m_currentCustomFee * m_estimatedTransactionSize) / 1000;
```

### Integration with SendCoinsDialog

#### Button Addition
- **Location**: Added next to existing "Choose..." fee button
- **Icon**: Uses platform style icons when available
- **Tooltip**: Clear description of functionality

#### Integration Logic
```cpp
void SendCoinsDialog::onCustomFeeSelectionClicked()
{
    // Open custom fee selection dialog
    CustomFeeSelectionDialog dlg(platformStyle, this);
    dlg.setModel(model);
    
    // Set current values from existing fee settings
    if (ui->radioSmartFee->isChecked()) {
        dlg.setCurrentMode(CustomFeeSelectionDialog::Standard);
    } else {
        dlg.setCurrentMode(CustomFeeSelectionDialog::Custom);
        dlg.setCurrentCustomFee(ui->customFee->value());
    }
    
    // Apply selected settings back to SendCoinsDialog
    if (dlg.exec() == QDialog::Accepted) {
        // Update fee radio buttons and values
        // Update confirmation target
        // Refresh fee display
    }
}
```

## User Experience Features

### Mode Switching
- **Seamless Transition**: Easy toggle between Basic and Advanced modes
- **Context Awareness**: Advanced mode automatically selects custom fee when enabled
- **Visual Feedback**: Clear indication of current mode and settings

### Fee Validation
- **Minimum Fee Check**: Validates against network minimum relay fee
- **Warning System**: Visual warnings for potentially problematic fees
- **Confirmation Dialog**: Warns user before accepting low fees

### Settings Persistence
- **User Preferences**: Saves advanced mode toggle state
- **Last Used Settings**: Remembers last custom fee and mode
- **Default Restoration**: Easy reset to standard settings

## Benefits

### For Users
- **Simplified Fee Selection**: Basic mode for common use cases
- **Advanced Control**: Custom fee input for power users
- **Cost Optimization**: Eco mode for cost-conscious users
- **Speed Control**: High priority for urgent transactions
- **Transparency**: Clear fee breakdown and confirmation estimates

### For Developers
- **Modular Design**: Clean separation of fee selection logic
- **Extensible Architecture**: Easy to add new fee modes
- **Backward Compatibility**: No breaking changes to existing code
- **Performance**: Efficient fee calculation and updates
- **Maintainability**: Well-documented and structured code

## Future Enhancements

### Potential Improvements
1. **Dynamic Fee Estimation**: Real-time fee estimation based on mempool
2. **Fee History**: Track fee trends and suggest optimal fees
3. **Batch Fee Calculation**: Calculate fees for multiple transactions
4. **Fee Presets**: User-defined fee presets for common scenarios
5. **Network Fee Monitoring**: Real-time network fee statistics
6. **Fee Optimization**: AI-powered fee suggestions
7. **Fee Comparison**: Compare fees across different confirmation targets
8. **Fee Alerts**: Notifications for optimal fee conditions

### Technical Enhancements
1. **Async Fee Calculation**: Non-blocking fee estimation
2. **Fee Caching**: Cache fee estimates for better performance
3. **Network Integration**: Direct integration with fee estimation APIs
4. **Advanced Validation**: More sophisticated fee validation rules
5. **Custom Fee Algorithms**: User-defined fee calculation methods

## Testing Considerations

### Manual Testing
1. **Mode Switching**: Test Basic/Advanced mode transitions
2. **Fee Calculation**: Verify fee calculations for all modes
3. **Validation**: Test fee validation and warnings
4. **Integration**: Test integration with SendCoinsDialog
5. **Persistence**: Verify settings are saved and restored
6. **Performance**: Test with large transaction sizes
7. **Edge Cases**: Test boundary conditions and error states

### Automated Testing
1. **Unit Tests**: Test fee calculation methods
2. **Integration Tests**: Test dialog integration
3. **UI Tests**: Test user interactions and feedback
4. **Validation Tests**: Test fee validation logic

## Success Metrics

### User Adoption
- **Usage Rate**: Percentage of users using custom fee selection
- **Mode Preference**: Distribution between Basic and Advanced modes
- **Feature Discovery**: Time to discover and use the feature

### User Satisfaction
- **Transaction Success Rate**: Success rate of transactions with custom fees
- **Fee Optimization**: Average fee savings achieved
- **User Feedback**: Positive feedback and feature requests

### Technical Performance
- **Response Time**: Speed of fee calculation and dialog opening
- **Memory Usage**: Efficient memory usage for fee calculations
- **Error Rate**: Low error rate in fee validation and application

## Conclusion

The Custom Fee Selection UI provides a comprehensive solution for transaction fee management in the Shahcoin Qt Wallet. By offering both Basic and Advanced modes, it caters to users of all experience levels while maintaining the flexibility and control that advanced users require. The implementation is well-structured, extensible, and integrates seamlessly with the existing wallet functionality.
