### SOLID Principles Explained with System Implementation  

#### 1. **Single Responsibility Principle (SRP)**  
**Definition**: A class should have only one reason to change, meaning it should have only one job.  
**Implementation in System**:  
- `RideManager`: Handles *only* ride lifecycle (creation → assignment → completion)  
- `FareCalculator`: Dedicated *exclusively* to fare calculation logic  
- `Notifier`: Manages *only* observer notifications  
- **Why**: Prevents god classes, simplifies maintenance  
- **Trade-off**: `Ride` combines state + notifications (acceptable for cohesion)  

---

#### 2. **Open/Closed Principle (OCP)**  
**Definition**: Software entities should be open for extension but closed for modification.  
**Implementation in System**:  
- **Driver Matching**: New strategies (e.g., `EcoFriendlyStrategy`) implement `DriverMatchingStrategy` interface without modifying `RideManager`  
- **Fare Calculation**: New rules (e.g., `WeekendPricingDecorator`) extend `FareDecorator`  
- **Notifications**: New observers (e.g., `AnalyticsNotifier`) implement `RideObserver`  
- **Why**: Enables feature additions without altering tested core logic  

---

#### 3. **Liskov Substitution Principle (LSP)**  
**Definition**: Subtypes must be substitutable for their base types without altering correctness.  
**Implementation in System**:  
- **Matching Strategies**: `NearestDriverStrategy` and `BestRatingDriverStrategy` are interchangeable in `RideManager`  
- **Fare Calculators**: `BaseFareCalculator` and decorators share `FareCalculator` interface  
- **Why**: Guarantees algorithm flexibility and behavioral consistency  
- **Validation**: Unit tests verify identical method signatures  

---

#### 4. **Interface Segregation Principle (ISP)**  
**Definition**: Clients shouldn't be forced to depend on interfaces they don't use.  
**Implementation in System**:  
- `RideObserver`: Single `update()` method  
- `DriverMatchingStrategy`: Single `matchDriver()` method  
- `FareCalculator`: Single `calculateFare()` method  
- **Why**: Prevents "fat interfaces" and implementation bloat  

---

#### 5. **Dependency Inversion Principle (DIP)**  
**Definition**: Depend on abstractions, not concretions.  
**Implementation in System**:  
- `RideManager` depends on `DriverMatchingStrategy` *interface*  
- `Notifier` depends on `RideObserver` *interface*  
- **Why**: Decouples high-level policy from implementation details  
- **Benefit**: Enables runtime strategy swapping  

---

### Design Patterns Explained with System Implementation  

#### 1. **Strategy Pattern**  
**Definition**: Define algorithm families and make them interchangeable.  
**Implementation in System**:  
- **Where**: Driver matching algorithms  
- **Components**:  
  - `DriverMatchingStrategy` (interface)  
  - `NearestDriverStrategy` (find by proximity)  
  - `BestRatingDriverStrategy` (find by rating)  
- **Why**: Business can experiment with matching logic without code changes  
- **Usage**: `RideManager` switches strategies at runtime  

---

#### 2. **Observer Pattern**  
**Definition**: Define subscription mechanism to notify objects of events.  
**Implementation in System**:  
- **Where**: Ride status change notifications  
- **Components**:  
  - Subject: `Ride` (publishes state changes)  
  - Observers: `RiderNotifier`, `DriverNotifier`  
  - Notifier: Central dispatch  
- **Why**: Decouples ride workflow from notification delivery  
- **Benefit**: Add SMS/email notifiers without modifying `Ride`  

---

#### 3. **Decorator Pattern**  
**Definition**: Attach additional responsibilities to objects dynamically.  
**Implementation in System**:  
- **Where**: Fare calculation enhancements  
- **Components**:  
  - `BaseFareCalculator` (core logic)  
  - `SurgePricingDecorator` (multiplier)  
  - `DiscountDecorator` (fixed deduction)  
- **Why**: Combine pricing rules flexibly  
- **Example**: `Base → Surge(1.2x) → Discount($10)` stack  

---

#### 4. **Singleton Pattern**  
**Definition**: Ensure class has only one instance with global access.  
**Implementation in System**:  
- **Where**: Manager classes  
- **Components**:  
  - `RiderManager`, `DriverManager`, `RideManager`  
  - Accessed via `getInstance()`  
- **Why**: Centralize access to shared state  
- **Trade-off**: Global state complicates testing (mitigated by reset methods)  

---

#### 5. **Factory Pattern**  
**Definition**: Delegate object creation to specialized methods.  
**Implementation in System**:  
- **Where**: Object initialization  
- **Components**:  
  - `RideManager.createRide()` (encapsulates setup)  
  - `Notifier.attach()` (observer registration)  
- **Why**: Isolate complex creation logic  
- **Benefit**: Ensures proper observer attachment  

---

### Why This Design Works  

1. **Extensible Architecture**  
   - Add new matching strategies in <10 lines of code  
   - Introduce pricing rules without touching core calculator  

2. **Maintainable Codebase**  
   - 200-line class cap (except managers)  
   - Separation of concerns via packages  

3. **Business Flexibility**  
   - Runtime strategy switching  
   - Dynamic pricing experimentation  

4. **Real-world Readiness**  
   - Notification system mirrors production  
   - State machine for ride lifecycle  

5. **Constraint Compliance**  
   - Pure in-memory operation  
   - Zero external dependencies
---

### Main Function Workflow:

    int main() {

    // Initialize managers
    auto& riderManager = RiderManager::getInstance();
    auto& driverManager = DriverManager::getInstance();
    auto& rideManager = RideManager::getInstance();

    // Create sample data
    auto rider1 = make_shared<Rider>("R1", "Alice", "111-222", Location(0,0));
    auto driver1 = make_shared<Driver>("D1", "John", "555-666", 
        VehicleType::SEDAN, "DL123", Location(1,1), 4.8);
    
    // Register users
    riderManager.addRider(rider1);
    driverManager.addDriver(driver1);

    // Create ride
    BookingDetails details{Location(0,0), Location(10,10), VehicleType::SEDAN, RideType::NORMAL};
    auto ride = rideManager.createRide(rider1, details);

    // Assign driver (60% acceptance chance)
    if (rideManager.assignDriver(ride)) {
        rideManager.startRide(ride);
        
        // Calculate fare with 20% surge
        auto fareCalculator = make_unique<SurgePricingDecorator>(
            make_unique<BaseFareCalculator>(), 1.2);
        rideManager.completeRide(ride, move(fareCalculator));
    }
    return 0;
    }

---

### Output Flow Explanation:

===== Testing Nearest Driver Strategy =====

[Rider Notification] Ride RIDE_1 Status: CONFIRMED

Booking Details:

  /t Pickup: (0,0)
  
  Dropoff: (10,10)
  
  Vehicle Type: SEDAN
  
  Ride Type: Normal

Assigned Driver: Driver D1 (John): SEDAN, Rating: 4.8, Location: (1,1)

[Driver Notification] Ride RIDE_1 Status: CONFIRMED

... (same details as above)

[Rider Notification] Ride RIDE_1 Status: IN_PROGRESS

... (status updates throughout ride)

[Rider Notification] Ride RIDE_1 Status: COMPLETED

... (fare calculation details)

Fare: $260.13

===== Testing Best Rated Driver Strategy =====

... (similar flow with different strategy)
