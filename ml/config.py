"""
CitySync - Master Civic Taxonomy Configuration & Category Mapping
Defines the unified 9-category civic taxonomy, NYC 311 mappings, 
BBMP/Indian civic mappings, and municipal SLA resolution benchmarks.
"""

# ==============================================================================
# 1. UNIFIED MASTER CIVIC TAXONOMY (9 Standardized Categories)
# ==============================================================================
MASTER_CATEGORIES = [
    "Roads & Infrastructure",
    "Solid Waste Management",
    "Street Lighting & Electrical",
    "Drainage & Water Supply",
    "Public Health & Sanitation",
    "Animal Husbandry & Stray Animals",
    "Horticulture & Greenery",
    "Encroachments & Footpaths",
    "Revenue & Urban Governance",
]

DEFAULT_CATEGORY = "Roads & Infrastructure"

# ==============================================================================
# 2. NYC 311 COMPLAINT TYPE -> MASTER TAXONOMY MAPPING
# ==============================================================================
NYC_CATEGORY_MAPPING = {
    # Roads & Infrastructure
    "Street Condition": "Roads & Infrastructure",
    "Sidewalk Condition": "Roads & Infrastructure",
    "Highway Condition": "Roads & Infrastructure",
    "Curb Condition": "Roads & Infrastructure",
    "Bridge Condition": "Roads & Infrastructure",
    "Traffic": "Roads & Infrastructure",
    "Bus Stop Shelter Placement": "Roads & Infrastructure",
    
    # Solid Waste Management
    "Dirty Condition": "Solid Waste Management",
    "Missed Collection": "Solid Waste Management",
    "Illegal Dumping": "Solid Waste Management",
    "Litter Basket / Request": "Solid Waste Management",
    "Recycling": "Solid Waste Management",
    "Sanitation Condition": "Solid Waste Management",
    "Sweeping/Inadequate": "Solid Waste Management",
    "Commercial Disposal Complaint": "Solid Waste Management",
    "Graffiti": "Solid Waste Management",
    
    # Street Lighting & Electrical
    "Street Light Condition": "Street Lighting & Electrical",
    "Traffic Signal Condition": "Street Lighting & Electrical",
    "ELECTRIC": "Street Lighting & Electrical",
    "Street Sign - Missing": "Street Lighting & Electrical",
    "Street Sign - Damaged": "Street Lighting & Electrical",
    "Street Sign - Dangling": "Street Lighting & Electrical",
    
    # Drainage & Water Supply
    "Water System": "Drainage & Water Supply",
    "WATER LEAK": "Drainage & Water Supply",
    "Sewer": "Drainage & Water Supply",
    "Catch Basin": "Drainage & Water Supply",
    "PLUMBING": "Drainage & Water Supply",
    "Hydrant": "Drainage & Water Supply",
    "Water Quality": "Drainage & Water Supply",
    "Water Conservation": "Drainage & Water Supply",
    "Standing Water": "Drainage & Water Supply",
    
    # Public Health & Sanitation
    "UNSANITARY CONDITION": "Public Health & Sanitation",
    "Food Safety": "Public Health & Sanitation",
    "Air Quality": "Public Health & Sanitation",
    "Indoor Sewage": "Public Health & Sanitation",
    "Public Toilet": "Public Health & Sanitation",
    "Smoking or Vaping": "Public Health & Sanitation",
    "Mold": "Public Health & Sanitation",
    "Food Establishment": "Public Health & Sanitation",
    "Mobile Food Vendor": "Public Health & Sanitation",
    "Drinking Water": "Public Health & Sanitation",
    "Asbestos": "Public Health & Sanitation",
    "Hazardous Materials": "Public Health & Sanitation",
    
    # Animal Husbandry & Stray Animals
    "Rodent": "Animal Husbandry & Stray Animals",
    "Animal in a Park": "Animal Husbandry & Stray Animals",
    "Animal-Abuse": "Animal Husbandry & Stray Animals",
    "Illegal Animal Kept": "Animal Husbandry & Stray Animals",
    "Dead Animal": "Animal Husbandry & Stray Animals",
    "Unleashed Dog": "Animal Husbandry & Stray Animals",
    "Harboring Bees/Wasps": "Animal Husbandry & Stray Animals",
    
    # Horticulture & Greenery
    "Damaged Tree": "Horticulture & Greenery",
    "Overgrown Tree/Branches": "Horticulture & Greenery",
    "Park Maintenance": "Horticulture & Greenery",
    "Dead/Dying Tree": "Horticulture & Greenery",
    "Root/Sewer/Sidewalk Condition": "Horticulture & Greenery",
    "Wood Disposal Form": "Horticulture & Greenery",
    "New Tree Request": "Horticulture & Greenery",
    
    # Encroachments & Footpaths
    "Encampment": "Encroachments & Footpaths",
    "Vendor Enforcement": "Encroachments & Footpaths",
    "Sidewalk Encroachment": "Encroachments & Footpaths",
    "Illegal Posting": "Encroachments & Footpaths",
    "Blocked Driveway": "Encroachments & Footpaths",
    "Illegal Parking": "Encroachments & Footpaths",
    "Abandoned Vehicle": "Encroachments & Footpaths",
    "Derelict Vehicles": "Encroachments & Footpaths",
    
    # Revenue & Urban Governance
    "Building/Use": "Revenue & Urban Governance",
    "Maintenance or Facility": "Revenue & Urban Governance",
    "General Construction/Plumbing": "Revenue & Urban Governance",
    "Elevator": "Revenue & Urban Governance",
    "Boiler": "Revenue & Urban Governance",
    "HEAT/HOT WATER": "Revenue & Urban Governance",
    "DOOR/WINDOW": "Revenue & Urban Governance",
    "FLOORING/STAIRS": "Revenue & Urban Governance",
    "PAINT/PLASTER": "Revenue & Urban Governance",
    "APPLIANCE": "Revenue & Urban Governance",
    "GENERAL": "Revenue & Urban Governance",
    "Noise": "Public Health & Sanitation",
    "Noise - Residential": "Public Health & Sanitation",
    "Noise - Commercial": "Public Health & Sanitation",
    "Noise - Street/Sidewalk": "Public Health & Sanitation",
    "Noise - Vehicle": "Public Health & Sanitation",
    "Noise - Helicopter": "Public Health & Sanitation"
}

# ==============================================================================
# 3. INDIAN BBMP GRIEVANCES -> MASTER TAXONOMY MAPPING
# ==============================================================================
BBMP_CATEGORY_MAPPING = {
    "Solid Waste (Garbage) Related": "Solid Waste Management",
    "Electrical": "Street Lighting & Electrical",
    "Road Maintenance(Engg)": "Roads & Infrastructure",
    "Road Infrastructure": "Roads & Infrastructure",
    "Forest": "Horticulture & Greenery",
    "Parks and Play grounds": "Horticulture & Greenery",
    "veterinary": "Animal Husbandry & Stray Animals",
    "Health Dept": "Public Health & Sanitation",
    "Storm  Water Drain(SWD)": "Drainage & Water Supply",
    "Storm Water Drain": "Drainage & Water Supply",
    "Lakes": "Drainage & Water Supply",
    "OFC & Water Supply (BWSSB)": "Drainage & Water Supply",
    "E khata / Khata services": "Revenue & Urban Governance",
    "Revenue Department": "Revenue & Urban Governance",
    "Town Planning": "Revenue & Urban Governance",
    "Advertisement": "Revenue & Urban Governance",
    "BBMP Property Tax": "Revenue & Urban Governance",
    "Encroachments": "Encroachments & Footpaths"
}

# ==============================================================================
# 4. INDIAN MUNICIPAL CITIZEN CHARTER SLA BENCHMARKS (Resolution Hours)
# ==============================================================================
INDIAN_MUNICIPAL_SLAS = {
    "Street Lighting & Electrical": {
        "target_hours": 24.0,
        "min_hours": 4.0,
        "max_hours": 48.0,
        "priority": "High",
        "sla_tier": "Immediate (Within 24 Hours)"
    },
    "Drainage & Water Supply": {
        "target_hours": 36.0,
        "min_hours": 6.0,
        "max_hours": 72.0,
        "priority": "High",
        "sla_tier": "Urgent (24 to 48 Hours)"
    },
    "Solid Waste Management": {
        "target_hours": 24.0,
        "min_hours": 6.0,
        "max_hours": 48.0,
        "priority": "Medium",
        "sla_tier": "Immediate (Within 24 Hours)"
    },
    "Roads & Infrastructure": {
        "target_hours": 48.0,
        "min_hours": 12.0,
        "max_hours": 96.0,
        "priority": "High",
        "sla_tier": "Standard (24 to 72 Hours)"
    },
    "Animal Husbandry & Stray Animals": {
        "target_hours": 36.0,
        "min_hours": 12.0,
        "max_hours": 72.0,
        "priority": "Medium",
        "sla_tier": "Standard (24 to 48 Hours)"
    },
    "Public Health & Sanitation": {
        "target_hours": 48.0,
        "min_hours": 12.0,
        "max_hours": 96.0,
        "priority": "Medium",
        "sla_tier": "Standard (24 to 72 Hours)"
    },
    "Horticulture & Greenery": {
        "target_hours": 48.0,
        "min_hours": 12.0,
        "max_hours": 120.0,
        "priority": "Low",
        "sla_tier": "Standard (24 to 72 Hours)"
    },
    "Encroachments & Footpaths": {
        "target_hours": 72.0,
        "min_hours": 24.0,
        "max_hours": 168.0,
        "priority": "Medium",
        "sla_tier": "Extended (3 to 7 Days)"
    },
    "Revenue & Urban Governance": {
        "target_hours": 120.0,
        "min_hours": 48.0,
        "max_hours": 240.0,
        "priority": "Low",
        "sla_tier": "Administrative (5 to 10 Days)"
    }
}
