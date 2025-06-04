#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <memory>
#include <algorithm>
#include <sstream>
#include <limits>
#include <iomanip>
#include <cstdlib>
#include <ctime>

// Enums and Constants
enum class VehicleType { BIKE, SEDAN, SUV, AUTO_RICKSHAW };
enum class RideStatus { REQUESTED, CONFIRMED, IN_PROGRESS, COMPLETED };
enum class RideType { NORMAL, CARPOOL };
enum class DriverStatus { AVAILABLE, ON_TRIP };

// Helper functions
std::string rideStatusToString(RideStatus status) {
    switch(status) {
        case RideStatus::REQUESTED: return "REQUESTED";
        case RideStatus::CONFIRMED: return "CONFIRMED";
        case RideStatus::IN_PROGRESS: return "IN_PROGRESS";
        case RideStatus::COMPLETED: return "COMPLETED";
        default: return "UNKNOWN";
    }
}

std::string vehicleTypeToString(VehicleType type) {
    switch(type) {
        case VehicleType::BIKE: return "BIKE";
        case VehicleType::SEDAN: return "SEDAN";
        case VehicleType::SUV: return "SUV";
        case VehicleType::AUTO_RICKSHAW: return "AUTO_RICKSHAW";
        default: return "UNKNOWN";
    }
}

// Location
class Location {
private:
    double x, y;
public:
    Location(double x, double y) : x(x), y(y) {}
    double distanceTo(const Location& other) const {
        return std::sqrt(std::pow(x - other.x, 2) + std::pow(y - other.y, 2));
    }
    std::string toString() const {
        return "(" + std::to_string((int)x) + ", " + std::to_string((int)y) + ")";
    }
};

// User
class User {
protected:
    std::string id, name, phone;
public:
    User(std::string id, std::string name, std::string phone) 
        : id(id), name(name), phone(phone) {}
    std::string getId() const { return id; }
    std::string getName() const { return name; }
    virtual ~User() = default;
};

class Rider : public User {
private:
    Location location;
public:
    Rider(std::string id, std::string name, std::string phone, Location loc)
        : User(id, name, phone), location(loc) {}
    Location getLocation() const { return location; }
};

class Driver : public User {
private:
    VehicleType vehicleType;
    std::string licensePlate;
    Location location;
    DriverStatus status;
    double rating;
public:
    Driver(std::string id, std::string name, std::string phone, 
           VehicleType vType, std::string plate, Location loc, double rating)
        : User(id, name, phone), vehicleType(vType), licensePlate(plate), 
          location(loc), status(DriverStatus::AVAILABLE), rating(rating) {}

    bool isAvailable() const { return status == DriverStatus::AVAILABLE; }
    Location getLocation() const { return location; }
    double getRating() const { return rating; }
    VehicleType getVehicleType() const { return vehicleType; }
    void setStatus(DriverStatus s) { status = s; }
    void printDetails() const {
        std::cout << "Driver " << id << " (" << name << "): " 
                  << vehicleTypeToString(vehicleType) << ", Rating: " 
                  << std::fixed << std::setprecision(1) << rating
                  << ", Location: " << location.toString() 
                  << ", Status: " << (status == DriverStatus::AVAILABLE ? "Available" : "On Trip")
                  << std::endl;
    }
};

// BookingDetails
struct BookingDetails {
    Location pickup;
    Location dropoff;
    VehicleType vehicleType;
    RideType rideType;
    
    void printDetails() const {
        std::cout << "Booking Details:\n"
                  << "  Pickup: " << pickup.toString() << "\n"
                  << "  Dropoff: " << dropoff.toString() << "\n"
                  << "  Vehicle Type: " << vehicleTypeToString(vehicleType) << "\n"
                  << "  Ride Type: " << (rideType == RideType::NORMAL ? "Normal" : "Carpool") << "\n";
    }
};

// Forward declaration
class Ride;

// Observer Interface
class RideObserver {
public:
    virtual void update(const Ride* ride) = 0;
    virtual ~RideObserver() = default;
};

class RiderNotifier : public RideObserver {
public:
    void update(const Ride* ride) override;
};

class DriverNotifier : public RideObserver {
public:
    void update(const Ride* ride) override;
};

class Notifier {
private:
    std::vector<std::shared_ptr<RideObserver>> observers;
public:
    void attach(std::shared_ptr<RideObserver> observer) {
        observers.push_back(observer);
    }
    void notify(const Ride* ride) {
        for (auto& obs : observers) obs->update(ride);
    }
};

class Ride {
private:
    std::string id;
    std::shared_ptr<Rider> rider;
    std::shared_ptr<Driver> driver;
    BookingDetails details;
    RideStatus status;
    double fare;
    Notifier notifier;
public:
    Ride(std::string id, std::shared_ptr<Rider> rider, BookingDetails details)
        : id(id), rider(std::move(rider)), driver(nullptr), 
          details(details), status(RideStatus::REQUESTED), fare(0.0) {}

    void assignDriver(std::shared_ptr<Driver> drv) {
        driver = drv;
        status = RideStatus::CONFIRMED;
        notifier.notify(this);
    }

    void startRide() {
        status = RideStatus::IN_PROGRESS;
        notifier.notify(this);
    }

    void completeRide(double fareAmount) {
        status = RideStatus::COMPLETED;
        fare = fareAmount;
        notifier.notify(this);
    }

    void attachObserver(std::shared_ptr<RideObserver> observer) {
        notifier.attach(observer);
    }

    std::string getId() const { return id; }
    RideStatus getStatus() const { return status; }
    std::shared_ptr<Driver> getDriver() const { return driver; }
    std::shared_ptr<Rider> getRider() const { return rider; }
    const BookingDetails& getDetails() const { return details; }
    double getFare() const { return fare; }
    
    void printDetails() const {
        std::cout << "Ride " << id << " Status: " << rideStatusToString(status) << "\n";
        details.printDetails();
        if (driver) {
            std::cout << "Assigned Driver: ";
            driver->printDetails();
        }
        if (status == RideStatus::COMPLETED) {
            std::cout << "Fare: $" << std::fixed << std::setprecision(2) << fare << "\n";
        }
    }
};

void RiderNotifier::update(const Ride* ride) {
    std::cout << "[Rider Notification] ";
    ride->printDetails();
}

void DriverNotifier::update(const Ride* ride) {
    std::cout << "[Driver Notification] ";
    ride->printDetails();
}

// Strategy Pattern for Driver Matching
class DriverMatchingStrategy {
public:
    virtual std::shared_ptr<Driver> matchDriver(const Location& pickup, 
                                              const std::vector<std::shared_ptr<Driver>>& drivers) = 0;
    virtual ~DriverMatchingStrategy() = default;
};

class NearestDriverStrategy : public DriverMatchingStrategy {
public:
    std::shared_ptr<Driver> matchDriver(const Location& pickup, 
                                      const std::vector<std::shared_ptr<Driver>>& drivers) override {
        std::shared_ptr<Driver> nearest = nullptr;
        double minDist = std::numeric_limits<double>::max();
        for (const auto& driver : drivers) {
            double dist = pickup.distanceTo(driver->getLocation());
            if (dist < minDist) {
                minDist = dist;
                nearest = driver;
            }
        }
        return nearest;
    }
};

class BestRatingDriverStrategy : public DriverMatchingStrategy {
public:
    std::shared_ptr<Driver> matchDriver(const Location& pickup, 
                                      const std::vector<std::shared_ptr<Driver>>& drivers) override {
        std::shared_ptr<Driver> best = nullptr;
        double maxRating = 0.0;
        for (const auto& driver : drivers) {
            if (driver->getRating() > maxRating) {
                maxRating = driver->getRating();
                best = driver;
            }
        }
        return best;
    }
};

// Fare Calculation
class FareCalculator {
public:
    virtual double calculateFare(const Ride* ride) = 0;
    virtual ~FareCalculator() = default;
};

class BaseFareCalculator : public FareCalculator {
public:
    double calculateFare(const Ride* ride) override {
        const BookingDetails& details = ride->getDetails();
        double baseFare, perKmRate;
        
        if (details.rideType == RideType::CARPOOL) {
            baseFare = 30.0;
            perKmRate = 10.0;
        } else {
            baseFare = 40.0;
            perKmRate = 12.5;
        }
        
        double dist = details.pickup.distanceTo(details.dropoff);
        return baseFare + (dist * perKmRate);
    }
};

class FareDecorator : public FareCalculator {
protected:
    std::unique_ptr<FareCalculator> component;
public:
    FareDecorator(std::unique_ptr<FareCalculator> comp) : component(std::move(comp)) {}
};

class SurgePricingDecorator : public FareDecorator {
private:
    double multiplier;
public:
    SurgePricingDecorator(std::unique_ptr<FareCalculator> comp, double mult)
        : FareDecorator(std::move(comp)), multiplier(mult) {}
    double calculateFare(const Ride* ride) override {
        return multiplier * component->calculateFare(ride);
    }
};

class DiscountDecorator : public FareDecorator {
private:
    double discount;
public:
    DiscountDecorator(std::unique_ptr<FareCalculator> comp, double disc)
        : FareDecorator(std::move(comp)), discount(disc) {}
    double calculateFare(const Ride* ride) override {
        return std::max(0.0, component->calculateFare(ride) - discount);
    }
};

// Manager Classes (Singletons)
class RiderManager {
private:
    std::map<std::string, std::shared_ptr<Rider>> riders;
    RiderManager() = default;
public:
    static RiderManager& getInstance() {
        static RiderManager instance;
        return instance;
    }
    void addRider(std::shared_ptr<Rider> rider) {
        riders[rider->getId()] = rider;
    }
    std::shared_ptr<Rider> getRider(const std::string& id) const {
        auto it = riders.find(id);
        return (it != riders.end()) ? it->second : nullptr;
    }
};

class DriverManager {
private:
    std::map<std::string, std::shared_ptr<Driver>> drivers;
    DriverManager() = default;
public:
    static DriverManager& getInstance() {
        static DriverManager instance;
        return instance;
    }
    void addDriver(std::shared_ptr<Driver> driver) {
        drivers[driver->getId()] = driver;
    }
    std::vector<std::shared_ptr<Driver>> getAvailableDrivers(VehicleType type) const {
        std::vector<std::shared_ptr<Driver>> available;
        for (const auto& [id, driver] : drivers) {
            if (driver->isAvailable() && driver->getVehicleType() == type) {
                available.push_back(driver);
            }
        }
        return available;
    }
    std::shared_ptr<Driver> getDriver(const std::string& id) const {
        auto it = drivers.find(id);
        return (it != drivers.end()) ? it->second : nullptr;
    }
};

class RideManager {
private:
    std::map<std::string, std::shared_ptr<Ride>> rides;
    std::unique_ptr<DriverMatchingStrategy> matchingStrategy;
    int rideCounter = 0;
    RideManager() : matchingStrategy(std::make_unique<NearestDriverStrategy>()) {}
public:
    static RideManager& getInstance() {
        static RideManager instance;
        return instance;
    }
    void setMatchingStrategy(std::unique_ptr<DriverMatchingStrategy> strategy) {
        matchingStrategy = std::move(strategy);
    }
    std::shared_ptr<Ride> createRide(std::shared_ptr<Rider> rider, BookingDetails details) {
        std::string rideId = "RIDE_" + std::to_string(++rideCounter);
        auto ride = std::make_shared<Ride>(rideId, rider, details);
        ride->attachObserver(std::make_shared<RiderNotifier>());
        ride->attachObserver(std::make_shared<DriverNotifier>());
        rides[rideId] = ride;
        return ride;
    }
    bool assignDriver(std::shared_ptr<Ride> ride) {
        auto availableDrivers = DriverManager::getInstance()
            .getAvailableDrivers(ride->getDetails().vehicleType);
        
        while (!availableDrivers.empty()) {
            auto matchedDriver = matchingStrategy->matchDriver(
                ride->getDetails().pickup, availableDrivers);
            
            if (!matchedDriver) break;
            
            // Simulate driver acceptance (60% chance)
            if (rand() % 100 < 60) {  
                ride->assignDriver(matchedDriver);
                matchedDriver->setStatus(DriverStatus::ON_TRIP);
                return true;
            } else {
                // Remove rejected driver from pool
                availableDrivers.erase(
                    std::remove(availableDrivers.begin(), availableDrivers.end(), matchedDriver),
                    availableDrivers.end());
                std::cout << "Driver " << matchedDriver->getId() 
                          << " rejected the ride. Trying next driver...\n";
            }
        }
        return false;
    }
    void startRide(std::shared_ptr<Ride> ride) {
        ride->startRide();
    }
    void completeRide(std::shared_ptr<Ride> ride, std::unique_ptr<FareCalculator> fareCalculator) {
        double fare = fareCalculator->calculateFare(ride.get());
        ride->completeRide(fare);
        if (auto driver = ride->getDriver()) {
            driver->setStatus(DriverStatus::AVAILABLE);
        }
    }
};

// Main Program
int main() {
    std::srand(std::time(0)); // Seed for random driver rejection
    
    auto& riderManager = RiderManager::getInstance();
    auto& driverManager = DriverManager::getInstance();
    auto& rideManager = RideManager::getInstance();

    // Create sample riders
    auto rider1 = std::make_shared<Rider>("R1", "Alice", "111-222", Location(0, 0));
    auto rider2 = std::make_shared<Rider>("R2", "Bob", "333-444", Location(5, 5));
    riderManager.addRider(rider1);
    riderManager.addRider(rider2);

    // Create sample drivers
    auto driver1 = std::make_shared<Driver>("D1", "John", "555-666", 
        VehicleType::SEDAN, "DL123", Location(1, 1), 4.8);
    auto driver2 = std::make_shared<Driver>("D2", "Mike", "777-888", 
        VehicleType::SUV, "DL456", Location(10, 10), 4.9);
    auto driver3 = std::make_shared<Driver>("D3", "Sarah", "999-000", 
        VehicleType::SEDAN, "DL789", Location(2, 2), 4.95);
    driverManager.addDriver(driver1);
    driverManager.addDriver(driver2);
    driverManager.addDriver(driver3);

    // Test with nearest driver strategy (default)
    std::cout << "\n===== Testing Nearest Driver Strategy =====\n";
    BookingDetails normalRideDetails{
        Location(0, 0),
        Location(10, 10),
        VehicleType::SEDAN,
        RideType::NORMAL
    };
    
    auto ride1 = rideManager.createRide(rider1, normalRideDetails);
    
    if (rideManager.assignDriver(ride1)) {
        rideManager.startRide(ride1);
        
        // Complete ride with base fare + 20% surge
        std::unique_ptr<FareCalculator> fareCalculator = 
            std::make_unique<SurgePricingDecorator>(
                std::make_unique<BaseFareCalculator>(), 1.2);
        rideManager.completeRide(ride1, std::move(fareCalculator));
    } else {
        std::cout << "No drivers available for ride!\n";
    }

    // Test with best-rated driver strategy
    std::cout << "\n===== Testing Best Rated Driver Strategy =====\n";
    rideManager.setMatchingStrategy(std::make_unique<BestRatingDriverStrategy>());
    
    BookingDetails carpoolDetails{
        Location(5, 5),
        Location(15, 15),
        VehicleType::SEDAN,
        RideType::CARPOOL
    };
    
    auto ride2 = rideManager.createRide(rider2, carpoolDetails);
    
    if (rideManager.assignDriver(ride2)) {
        rideManager.startRide(ride2);
        
        // Complete ride with base fare + 10% discount
        std::unique_ptr<FareCalculator> fareCalculator = 
            std::make_unique<DiscountDecorator>(
                std::make_unique<BaseFareCalculator>(), 10.0);
        rideManager.completeRide(ride2, std::move(fareCalculator));
    } else {
        std::cout << "No drivers available for ride!\n";
    }

    return 0;
}